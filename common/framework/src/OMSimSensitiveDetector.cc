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
#include "OMSimTools.hh"
#include <G4EventManager.hh>

thread_local G4OpBoundaryProcess *OMSimSensitiveDetector::m_boundaryProcess = nullptr;

/**
 * @brief Constructor for OMSimSensitiveDetector.
 * @param p_name Name of the sensitive detector.
 * @param p_detectorType Type of the detector (e.g., PMT, VolumePhotonDetector).
 */
OMSimSensitiveDetector::OMSimSensitiveDetector(G4String p_name, DetectorType p_detectorType)
    : G4VSensitiveDetector(p_name), m_detectorType(p_detectorType), m_PMTResponse(nullptr), m_QEcut(OMSimCommandArgsTable::getInstance().get<bool>("efficiency_cut"))
{
}

/**
 * @brief Destructor for OMSimSensitiveDetector.
 */

OMSimSensitiveDetector::~OMSimSensitiveDetector()
{
  delete m_PMTResponse;
  m_PMTResponse = nullptr;
}

/**
 * @brief Fetches the boundary process for detecting boundary absorptions.
 *
 * Retrieves and stores the `G4OpBoundaryProcess` to check for photon detection
 * at boundaries. Logs an error if the process is not found.
 */
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

/**
 * @brief Sets the PMT response model.
 * @param p_response Pointer to the PMT response object.
 */
void OMSimSensitiveDetector::setPMTResponse(OMSimPMTResponse *p_response)
{
  m_PMTResponse = p_response;
}

/**
 * @brief Processes hits for optical photons in the detector.
 *
 * If the detector type is one of the perfect detectors (100% efficient),
 * the photon hit is registered directly. Otherwise, it checks for volume
 * or boundary absorption based on the detector type.
 *
 * @param p_step The current step information.
 * @param p_touchableHistory The history of touchable objects.
 * @return True if the photon hit was stored, false otherwise.
 */
G4bool OMSimSensitiveDetector::ProcessHits(G4Step *p_step, G4TouchableHistory *p_touchableHistory)
{
  // Return false if the track is not an optical photon
  if (p_step->GetTrack()->GetDefinition() != G4OpticalPhoton::Definition())
    return false;

  // Switch based on the detector type
  switch (m_detectorType)
  {
  case DetectorType::PMT:
    if (checkBoundaryAbsorption(p_step))
      return handlePMT(p_step, p_touchableHistory);
    return false;
  
  case DetectorType::BoundaryShellDetector:
    return handleShellDetector(p_step, p_touchableHistory);

  case DetectorType::PerfectPMT:
    return handlePMT(p_step, p_touchableHistory);

  case DetectorType::BoundaryPhotonDetector:
    if (checkBoundaryAbsorption(p_step))
      return handleGeneralPhotonDetector(p_step, p_touchableHistory);
    return false;

  case DetectorType::VolumePhotonDetector:
    if (checkVolumeAbsorption(p_step))
      return handleGeneralPhotonDetector(p_step, p_touchableHistory);
    return false;
    
  default:
    return false;
  }

  return false;
}

/**
 * @brief Retrieves photon information from a given step.
 *
 * @param p_step The current step information.
 * @return PhotonInfo struct containing the photon details.
 */
PhotonInfo OMSimSensitiveDetector::getPhotonInfo(G4Step *p_step)
{
  PhotonInfo info;
  G4Track *track = p_step->GetTrack();

  G4double h = 4.135667696E-15 * eV * s;
  G4double c = 2.99792458E17 * nm / s;
  G4double lEkin = track->GetKineticEnergy();
  info.eventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
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
  if (m_PMTResponse)
    info.PMTResponse = m_PMTResponse->processPhotocathodeHit(info.localPosition.x(), info.localPosition.y(), info.wavelength);
  else
    info.PMTResponse = OMSimPMTResponse::PMTPulse({0, 0, 0});
  return info;
}

/**
 * @brief Checks if the photon was absorbed in the volume.
 *
 * @param p_step The current step information.
 * @return True if the photon was absorbed, false otherwise.
 */
G4bool OMSimSensitiveDetector::checkVolumeAbsorption(G4Step *p_step)
{
  return p_step->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName() == "OpAbsorption";
}

/**
 * @brief Checks if the photon was detected at a boundary.
 * @param p_step The current step information.
 * @return True if the photon was detected at the boundary, false otherwise.
 */
G4bool OMSimSensitiveDetector::checkBoundaryAbsorption(G4Step *p_step)
{
  if (m_boundaryProcess == nullptr)
    fetchBoundaryProcess();

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
 * @brief Monte carlo if the photon was detected based on the detection probability.
 * @param p_efficiency The detection probability.
 * @return True if the photon was detected, false otherwise.
 */
bool OMSimSensitiveDetector::isPhotonDetected(double p_efficiency)
{
  return G4UniformRand() < p_efficiency;
}

/**
 * @brief Handles hits for PMT detectors.
 * @param p_step The current step information.
 * @param p_touchableHistory The history of touchable objects.
 * @return True if the hit was stored, false otherwise.
 */
G4bool OMSimSensitiveDetector::handlePMT(G4Step *p_step, G4TouchableHistory *p_touchableHistory)
{
  PhotonInfo info = getPhotonInfo(p_step);

  // if QE cut is enabled, check if photon is detected (using detection probability, if detail PMT is enabled)
  if (m_QEcut && !isPhotonDetected(info.PMTResponse.detectionProbability))
    return false;
  else if (m_QEcut)
    info.PMTResponse.detectionProbability = 1; // if QE cut is enabled, detection probability is 1 as photon was detected

  G4TouchableHandle touchable = p_step->GetPreStepPoint()->GetTouchableHandle();
  G4String name;
  int i = 0;
  do
  {
    name = touchable->GetVolume(i)->GetName();
    i++;
  } while (name.substr(0, 3) != "PMT");

  std::vector<G4String> numberPMTstring = Tools::splitStringByDelimiter(name, '_');
  info.pmtNumber = atoi(numberPMTstring.at(1));
  storePhotonHit(info);
  killParticle(p_step->GetTrack());
  return true;
}

/**
 * @brief Handles hits for general photon detectors.
 * @param p_step The current step information.
 * @param p_touchableHistory The history of touchable objects.
 * @return True if the hit was stored, always true for general photon detectors.
 */
G4bool OMSimSensitiveDetector::handleGeneralPhotonDetector(G4Step *p_step, G4TouchableHistory *p_touchableHistory)
{
  PhotonInfo info = getPhotonInfo(p_step);
  info.pmtNumber = 0; // placeholder
  storePhotonHit(info);
  killParticle(p_step->GetTrack());
  return true;
}
/**
 * @brief Handles hits for shell photon detectors. This detector does not kill the particle.
 * @param p_step The current step information.
 * @param p_touchableHistory The history of touchable objects.
 * @return True if the hit was stored.alignas
 */
G4bool OMSimSensitiveDetector::handleShellDetector(G4Step *p_step, G4TouchableHistory *p_touchableHistory)

{

  G4StepPoint* postStepPoint = p_step->GetPostStepPoint();



  // Get the physical volume associated with the post-step point

  G4VPhysicalVolume* currentVolume = postStepPoint->GetPhysicalVolume();

  //log_info("Step passed to thehandleerrr  volume name {}, sensitive {}", currentVolume->GetName(), GetName());



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
      p_info.eventID,
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

/**
 * @brief Stop the particle from propagating further. Necessary for 100% efficient detectors.
 */
void OMSimSensitiveDetector::killParticle(G4Track *p_track)
{
  if (p_track->GetTrackStatus() != fStopAndKill)
  {
    p_track->SetTrackStatus(fStopAndKill);
  }
}
