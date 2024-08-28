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


thread_local G4OpBoundaryProcess* OMSimSensitiveDetector::m_boundaryProcess = nullptr;


std::vector<G4String> splitStringByDelimiter(G4String const &p_string, char p_delim)
{
  std::vector<G4String> result;
  std::istringstream iss(p_string);

  for (G4String token; std::getline(iss, token, p_delim);)
  {
    result.push_back(std::move(token));
  }

  return result;
}

std::vector<G4String> splitStringByDelimiter(char *p_char, char p_delim)
{
  return splitStringByDelimiter(G4String(p_char), p_delim);
}

/**
 * @brief Constructor.
 * @param p_name Name of the sensitive detector.
 * @param p_detectorType Type of the detector (e.g., PMT, VolumePhotonDetector).
 */
OMSimSensitiveDetector::OMSimSensitiveDetector(G4String p_name, DetectorType p_detectorType)
    : G4VSensitiveDetector(p_name), m_detectorType(p_detectorType), m_PMTResponse(&NoResponse::getInstance())
{
}

void OMSimSensitiveDetector::fetchBoundaryProcess()
{
  G4ProcessManager *processManager = G4OpticalPhoton::OpticalPhoton()->GetProcessManager();
  G4ProcessVector *processVector = processManager->GetProcessList();
  for (int i = 0; i < processVector->length(); i++)
  {
    if (dynamic_cast<G4OpBoundaryProcess *>((*processVector)[i]))
    {
      m_boundaryProcess = (G4OpBoundaryProcess *)(*processVector)[i];
      break;
    }
  }
  if (!m_boundaryProcess)
  {
    G4cerr << "Error: G4OpBoundaryProcess not found!" << G4endl;
  }
}

void OMSimSensitiveDetector::setPMTResponse(OMSimPMTResponse *p_response)
{
  m_PMTResponse = p_response;
}

/**
 * @brief Processes particles that go through detector.
 * @param p_step The current step information.
 * @param p_touchableHistory The history of touchable objects.
 * @return True if the particle was stored as hit, false otherwise.
 */
G4bool OMSimSensitiveDetector::ProcessHits(G4Step *p_step, G4TouchableHistory *p_touchableHistory)
{
  if (p_step->GetTrack()->GetDefinition() == G4OpticalPhoton::Definition()) // check if particle is a photon
  {
    if (checkVolumeAbsorption(p_step))
    {
      switch (m_detectorType)
      {
      case DetectorType::VolumePhotonDetector:
        return handleGeneralPhotonDetector(p_step, p_touchableHistory);
      }
    }
    else if (checkBoundaryAbsorption(p_step))
    {
      switch (m_detectorType)
      {
      case DetectorType::BoundaryPhotonDetector:
        return handleGeneralPhotonDetector(p_step, p_touchableHistory);
      case DetectorType::PMT:
        return handlePMT(p_step, p_touchableHistory);
      }
    }
  }

  return false;
}

PhotonInfo OMSimSensitiveDetector::getPhotonInfo(G4Step *p_step)
{
  PhotonInfo info;
  G4Track *track = p_step->GetTrack();

  G4double h = 4.135667696E-15 * eV * s;
  G4double c = 2.99792458E17 * nm / s;
  G4double lEkin = track->GetKineticEnergy();

  info.globalTime = track->GetGlobalTime();
  info.localTime = track->GetLocalTime();
  info.trackLength = track->GetTrackLength() / m;
  info.kineticEnergy = lEkin;
  info.wavelength = h * c / lEkin;
  info.globalPosition = track->GetPosition();
  info.localPosition = p_step->GetPostStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(info.globalPosition);
  info.momentumDirection = track->GetMomentumDirection();
  info.deltaPosition = track->GetVertexPosition() - info.globalPosition;
  info.detectorID = atoi(SensitiveDetectorName);
  info.PMTResponse = m_PMTResponse->processPhotocathodeHit(info.localPosition.x() / mm, info.localPosition.y() / mm, info.wavelength);
  return info;
}

G4bool OMSimSensitiveDetector::checkVolumeAbsorption(G4Step *p_step)
{
  return p_step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName() == "OpAbsorption";
}

G4bool OMSimSensitiveDetector::checkBoundaryAbsorption(G4Step *p_step)
{
  if (m_boundaryProcess == nullptr) fetchBoundaryProcess();

  if (p_step->GetPostStepPoint()->GetStepStatus() == fGeomBoundary)
  {
    if (m_boundaryProcess)
    {
      G4OpBoundaryProcessStatus boundaryStatus = m_boundaryProcess->GetStatus();
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
 * @param p_step The current step information.
 * @param p_touchableHistory The history of touchable objects.
 * @return True if the hit was stored
 */
G4bool OMSimSensitiveDetector::handlePMT(G4Step *p_step, G4TouchableHistory *p_touchableHistory)
{
  PhotonInfo info = getPhotonInfo(p_step);
  if (OMSimCommandArgsTable::getInstance().get<bool>("QE_cut") && !m_PMTResponse->passQE(info.wavelength))
    return false;
  G4TouchableHandle touchable = p_step->GetPreStepPoint()->GetTouchableHandle();
  G4String name;
  int i = 0;
  do
  {
    name = touchable->GetVolume(i)->GetName();
    i++;
  } while (name.substr(0, 3) != "PMT");
  
  std::vector<G4String> numberPMTstring = splitStringByDelimiter(name, '_');
  info.pmtNumber = atoi(numberPMTstring.at(1));
  storePhotonHit(info);
  return true;
}

/**
 * @brief Handles hits for general photon detectors.
 * This is the same as handlePMT, but sets PMT number to 0, as otherwise the retrieval of the PMT number from the volume name would results in an error.
 * @param p_step The current step information.
 * @param p_touchableHistory The history of touchable objects.
 * @return True if the hit was stored (always)
 */
G4bool OMSimSensitiveDetector::handleGeneralPhotonDetector(G4Step *p_step, G4TouchableHistory *p_touchableHistory)
{
  PhotonInfo info = getPhotonInfo(p_step);
  info.pmtNumber = 0; // placeholder
  storePhotonHit(info);
  return true;
}

/**
 * @brief Stores photon hit information into the HitManager
 * @param p_info The photon hit information.
 */
void OMSimSensitiveDetector::storePhotonHit(PhotonInfo &p_info)
{
  OMSimHitManager &hitManager = OMSimHitManager::getInstance();
  hitManager.appendHitInfo(
      p_info.globalTime,
      p_info.localTime,
      p_info.trackLength,
      p_info.kineticEnergy / eV,
      p_info.pmtNumber,
      p_info.momentumDirection,
      p_info.globalPosition,
      p_info.localPosition,
      p_info.deltaPosition.mag() / m,
      p_info.PMTResponse,
      p_info.detectorID);
}