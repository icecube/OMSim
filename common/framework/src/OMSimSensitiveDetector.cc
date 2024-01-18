#include "OMSimSensitiveDetector.hh"

#include "G4ios.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"
#include "OMSimEventAction.hh"
#include "G4OpticalPhoton.hh"
#include "OMSimPMTResponse.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include "G4VProcess.hh"
#include "G4ProcessVector.hh"
#include "G4ProcessManager.hh"
std::vector<G4String> splitStringByDelimiter(G4String const &s, char delim)
{
  std::vector<G4String> result;
  std::istringstream iss(s);

  for (G4String token; std::getline(iss, token, delim);)
  {
    result.push_back(std::move(token));
  }

  return result;
}

std::vector<G4String> splitStringByDelimiter(char *cs, char d)
{
  return splitStringByDelimiter(G4String(cs), d);
}

/**
 * @brief Constructor.
 * @param name Name of the sensitive detector.
 * @param pDetectorType Type of the detector (e.g., PMT, VolumePhotonDetector).
 */
OMSimSensitiveDetector::OMSimSensitiveDetector(G4String name, DetectorType pDetectorType)
    : G4VSensitiveDetector(name), mDetectorType(pDetectorType), mPMTResponse(&NoResponse::getInstance())
{
}

void OMSimSensitiveDetector::fetchBoundaryProcess()
{
  G4ProcessManager *lProcessManager = G4OpticalPhoton::OpticalPhoton()->GetProcessManager();
  G4ProcessVector *lProcessVector = lProcessManager->GetProcessList();
  for (int i = 0; i < lProcessVector->length(); i++)
  {
    if (dynamic_cast<G4OpBoundaryProcess *>((*lProcessVector)[i]))
    {
      mBoundaryProcess = (G4OpBoundaryProcess *)(*lProcessVector)[i];
      break;
    }
  }
  if (!mBoundaryProcess)
  {
    G4cerr << "Error: G4OpBoundaryProcess not found!" << G4endl;
  }
}

void OMSimSensitiveDetector::setPMTResponse(OMSimPMTResponse *pResponse)
{
  mPMTResponse = pResponse;
}

/**
 * @brief Processes particles that go through detector.
 * @param pStep The current step information.
 * @param pTouchableHistory The history of touchable objects.
 * @return True if the particle was stored as hit, false otherwise.
 */
G4bool OMSimSensitiveDetector::ProcessHits(G4Step *pStep, G4TouchableHistory *pTouchableHistory)
{
  if (pStep->GetTrack()->GetDefinition() == G4OpticalPhoton::Definition()) // check if particle is a photon
  {
    if (checkVolumeAbsorption(pStep))
    {
      switch (mDetectorType)
      {
      case DetectorType::PMT:
        return handlePMT(pStep, pTouchableHistory);
      case DetectorType::VolumePhotonDetector:
        return handleGeneralPhotonDetector(pStep, pTouchableHistory);
      }
    }
    else if (checkBoundaryAbsorption(pStep))
    {
      switch (mDetectorType)
      {
      case DetectorType::BoundaryPhotonDetector:
        return handleGeneralPhotonDetector(pStep, pTouchableHistory);
      }
    }
  }

  return false;
}

PhotonInfo OMSimSensitiveDetector::getPhotonInfo(G4Step *aStep)
{
  PhotonInfo info;
  G4Track *aTrack = aStep->GetTrack();

  G4double h = 4.135667696E-15 * eV * s;
  G4double c = 2.99792458E17 * nm / s;
  G4double lEkin = aTrack->GetKineticEnergy();

  info.globalTime = aTrack->GetGlobalTime();
  info.localTime = aTrack->GetLocalTime();
  info.trackLength = aTrack->GetTrackLength() / m;
  info.kineticEnergy = lEkin;
  info.wavelength = h * c / lEkin;
  info.globalPosition = aTrack->GetPosition();
  info.localPosition = aStep->GetPostStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(info.globalPosition);
  info.momentumDirection = aTrack->GetMomentumDirection();
  info.deltaPosition = aTrack->GetVertexPosition() - info.globalPosition;
  info.detectorID = atoi(SensitiveDetectorName);
  info.PMTResponse = mPMTResponse->processPhotocathodeHit(info.localPosition.x() / mm, info.localPosition.y() / mm, info.wavelength);
  return info;
}


G4bool OMSimSensitiveDetector::checkVolumeAbsorption(G4Step *aStep)
{
  return aStep->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName() == "OpAbsorption";
}

G4bool OMSimSensitiveDetector::checkBoundaryAbsorption(G4Step *aStep)
{
  if (mBoundaryProcess == nullptr)
  {
    fetchBoundaryProcess();
  };

  if (aStep->GetPostStepPoint()->GetStepStatus() == fGeomBoundary)
  {
    if (mBoundaryProcess)
    {
      G4OpBoundaryProcessStatus boundaryStatus = mBoundaryProcess->GetStatus();

      if (boundaryStatus == G4OpBoundaryProcessStatus::Detection)
      {
        return true;
      };
    }
  }
  return false;
}

/**
 * @brief Handles hits for PMTs.
 * @param pStep The current step information.
 * @param pTouchableHistory The history of touchable objects.
 * @return True if the hit was stored
 */
G4bool OMSimSensitiveDetector::handlePMT(G4Step *aStep, G4TouchableHistory *pTouchableHistory)
{
  PhotonInfo info = getPhotonInfo(aStep);

  if (OMSimCommandArgsTable::getInstance().get<bool>("QE_cut") && !mPMTResponse->passQE(info.wavelength))
    return false;

  std::vector<G4String> lPMTNameNR = splitStringByDelimiter(aStep->GetPreStepPoint()->GetTouchableHandle()->GetVolume(2)->GetName(), '_');
  info.pmtNumber = atoi(lPMTNameNR.at(1));

  storePhotonHit(info);
  return true;
}

/**
 * @brief Handles hits for general photon detectors.
 * This is the same as handlePMT, but sets PMT number to 0, as otherwise the retrieval of the PMT number from the volume name would results in an error.
 * @param pStep The current step information.
 * @param pTouchableHistory The history of touchable objects.
 * @return True if the hit was stored (always)
 */
G4bool OMSimSensitiveDetector::handleGeneralPhotonDetector(G4Step *aStep, G4TouchableHistory *pTouchableHistory)
{
  PhotonInfo info = getPhotonInfo(aStep);
  info.pmtNumber = 0; // placeholder
  storePhotonHit(info);
  return true;
}

/**
 * @brief Stores photon hit information into the HitManager
 * @param info The photon hit information.
 */
void OMSimSensitiveDetector::storePhotonHit(PhotonInfo &info)
{
  OMSimHitManager &lHitManager = OMSimHitManager::getInstance();
  lHitManager.appendHitInfo(
      info.globalTime,
      info.localTime,
      info.trackLength,
      info.kineticEnergy / eV,
      info.pmtNumber,
      info.momentumDirection,
      info.globalPosition,
      info.localPosition,
      info.deltaPosition.mag() / m,
      info.PMTResponse,
      info.detectorID);
}