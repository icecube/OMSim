/**
 * @todo Test several optical modules of same type with new type
 */

#include "OMSimSensitiveDetector.hh"
#include "OMSimPMTResponse.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"

#include "G4ios.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4VTouchable.hh"
#include "G4OpticalPhoton.hh"
#include "G4VProcess.hh"
#include "G4ProcessVector.hh"
#include "G4ProcessManager.hh"

std::vector<G4String> splitStringByDelimiter(G4String const &pString, char pDelim)
{
  std::vector<G4String> result;
  std::istringstream iss(pString);

  for (G4String token; std::getline(iss, token, pDelim);)
  {
    result.push_back(std::move(token));
  }

  return result;
}

std::vector<G4String> splitStringByDelimiter(char *pChar, char pDelim)
{
  return splitStringByDelimiter(G4String(pChar), pDelim);
}

/**
 * @brief Constructor.
 * @param pName Name of the sensitive detector.
 * @param pDetectorType Type of the detector (e.g., PMT, VolumePhotonDetector).
 */
OMSimSensitiveDetector::OMSimSensitiveDetector(G4String pName, DetectorType pDetectorType)
    : G4VSensitiveDetector(pName), mDetectorType(pDetectorType), mPMTResponse(&NoResponse::getInstance())
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

PhotonInfo OMSimSensitiveDetector::getPhotonInfo(G4Step *pStep)
{
  PhotonInfo info;
  G4Track *lTrack = pStep->GetTrack();

  G4double h = 4.135667696E-15 * eV * s;
  G4double c = 2.99792458E17 * nm / s;
  G4double lEkin = lTrack->GetKineticEnergy();

  info.globalTime = lTrack->GetGlobalTime();
  info.localTime = lTrack->GetLocalTime();
  info.trackLength = lTrack->GetTrackLength() / m;
  info.kineticEnergy = lEkin;
  info.wavelength = h * c / lEkin;
  info.globalPosition = lTrack->GetPosition();
  info.localPosition = pStep->GetPostStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(info.globalPosition);
  info.momentumDirection = lTrack->GetMomentumDirection();
  info.deltaPosition = lTrack->GetVertexPosition() - info.globalPosition;
  info.detectorID = atoi(SensitiveDetectorName);
  info.PMTResponse = mPMTResponse->processPhotocathodeHit(info.localPosition.x() / mm, info.localPosition.y() / mm, info.wavelength);
  return info;
}

G4bool OMSimSensitiveDetector::checkVolumeAbsorption(G4Step *pStep)
{
  return pStep->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName() == "OpAbsorption";
}

G4bool OMSimSensitiveDetector::checkBoundaryAbsorption(G4Step *pStep)
{
  if (mBoundaryProcess == nullptr)
  {
    fetchBoundaryProcess();
  };

  if (pStep->GetPostStepPoint()->GetStepStatus() == fGeomBoundary)
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
G4bool OMSimSensitiveDetector::handlePMT(G4Step *pStep, G4TouchableHistory *pTouchableHistory)
{
  PhotonInfo lInfo = getPhotonInfo(pStep);

  if (OMSimCommandArgsTable::getInstance().get<bool>("QE_cut"))
  {
    if (mPMTResponse->passQE(lInfo.wavelength))
    {
      return false;
    }
  }

  std::vector<G4String> lPMTNameNR = splitStringByDelimiter(pStep->GetPreStepPoint()->GetTouchableHandle()->GetVolume(2)->GetName(), '_');
  lInfo.pmtNumber = atoi(lPMTNameNR.at(1));

  storePhotonHit(lInfo);
  return true;
}

/**
 * @brief Handles hits for general photon detectors.
 * This is the same as handlePMT, but sets PMT number to 0, as otherwise the retrieval of the PMT number from the volume name would results in an error.
 * @param pStep The current step information.
 * @param pTouchableHistory The history of touchable objects.
 * @return True if the hit was stored (always)
 */
G4bool OMSimSensitiveDetector::handleGeneralPhotonDetector(G4Step *pStep, G4TouchableHistory *pTouchableHistory)
{
  PhotonInfo lInfo = getPhotonInfo(pStep);
  lInfo.pmtNumber = 0; // placeholder
  storePhotonHit(lInfo);
  return true;
}

/**
 * @brief Stores photon hit information into the HitManager
 * @param info The photon hit information.
 */
void OMSimSensitiveDetector::storePhotonHit(PhotonInfo &pInfo)
{
  OMSimHitManager &lHitManager = OMSimHitManager::getInstance();
  lHitManager.appendHitInfo(
      pInfo.globalTime,
      pInfo.localTime,
      pInfo.trackLength,
      pInfo.kineticEnergy / eV,
      pInfo.pmtNumber,
      pInfo.momentumDirection,
      pInfo.globalPosition,
      pInfo.localPosition,
      pInfo.deltaPosition.mag() / m,
      pInfo.PMTResponse,
      pInfo.detectorID);
}