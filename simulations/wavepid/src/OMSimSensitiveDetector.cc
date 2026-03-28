/**
 * @file OMSimSensitiveDetector.cc
 * @brief Enhanced sensitive detector implementation with photon origin tracking.
 */
#include "OMSimSensitiveDetector.hh"
#include "OMSimTrackingAction.hh"
#include "OMSimPMTResponse.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include "OMSimTools.hh"
#include "OMSimLogger.hh"

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
#include "G4EventManager.hh"
#include "OMSimOpBoundaryProcess.hh"
#include "G4SystemOfUnits.hh"

thread_local G4OpBoundaryProcess* OMSimSensitiveDetector::m_boundaryProcess = nullptr;

OMSimSensitiveDetector::OMSimSensitiveDetector(G4String pName, DetectorType pDetectorType)
    : G4VSensitiveDetector(pName),
      m_detectorType(pDetectorType),
      m_PMTResponse(nullptr),
      m_QEcut(OMSimCommandArgsTable::getInstance().get<bool>("efficiency_cut"))
{
}

OMSimSensitiveDetector::~OMSimSensitiveDetector()
{
    delete m_PMTResponse;
    m_PMTResponse = nullptr;
}

void OMSimSensitiveDetector::fetchBoundaryProcess()
{
    G4ProcessManager* processManager = G4OpticalPhoton::OpticalPhoton()->GetProcessManager();
    G4ProcessVector* processVector = processManager->GetProcessList();
    for (int i = 0; i < processVector->length(); i++) {
        if (dynamic_cast<G4OpBoundaryProcess*>((*processVector)[i])) {
            m_boundaryProcess = (G4OpBoundaryProcess*)(*processVector)[i];
            break;
        }
    }
    if (!m_boundaryProcess) {
        G4cerr << "Error: G4OpBoundaryProcess not found!" << G4endl;
    }
}

void OMSimSensitiveDetector::setPMTResponse(OMSimPMTResponse* pResponse)
{
    m_PMTResponse = pResponse;
}

G4bool OMSimSensitiveDetector::ProcessHits(G4Step* pStep, G4TouchableHistory* pTouchableHistory)
{
    // Only process optical photons
    if (pStep->GetTrack()->GetDefinition() != G4OpticalPhoton::Definition())
        return false;

    G4Track* track = pStep->GetTrack();

    // Memory optimization: only store photons within time window
    static constexpr G4double kMaxStoreTime = 500.0 * ns;
    if (track->GetGlobalTime() > kMaxStoreTime) {
        killParticle(track);
        return false;
    }

    // Memory optimization: only store photons within wavelength band
    static constexpr G4double kLambdaMin_nm = 300.0;
    static constexpr G4double kLambdaMax_nm = 650.0;
    const G4double lambda_nm = 1239.84193 / (track->GetKineticEnergy() / eV);
    if (lambda_nm < kLambdaMin_nm || lambda_nm > kLambdaMax_nm) {
        killParticle(track);
        return false;
    }

    switch (m_detectorType) {
    case DetectorType::PMT:
        if (checkBoundaryAbsorption(pStep))
            return handlePMT(pStep, pTouchableHistory);
        return false;

    case DetectorType::PerfectPMT:
        return handlePMT(pStep, pTouchableHistory);

    case DetectorType::BoundaryPhotonDetector:
        if (checkBoundaryAbsorption(pStep))
            return handleGeneralPhotonDetector(pStep, pTouchableHistory);
        return false;

    case DetectorType::VolumePhotonDetector:
        if (checkVolumeAbsorption(pStep))
            return handleGeneralPhotonDetector(pStep, pTouchableHistory);
        return false;

    case DetectorType::BoundaryShellDetector:
        return handleShellDetector(pStep, pTouchableHistory);

    default:
        return false;
    }

    return false;
}

PhotonInfo OMSimSensitiveDetector::getPhotonInfo(G4Step* pStep)
{
    PhotonInfo info;
    G4Track* track = pStep->GetTrack();

    // Get creator process name
    G4String creatorProcessName;
    G4String motherParticleName = "";
    G4String motherProcessName = "";

    const G4VProcess* creatorProcess = track->GetCreatorProcess();
    if (creatorProcess) {
        creatorProcessName = creatorProcess->GetProcessName();
    } else {
        creatorProcessName = "Primary";
    }

    // Get parent particle info using TrackingAction singleton
    G4int parentTrackID = track->GetParentID();
    std::string parentParticleType;

    if (parentTrackID == 0) {
        // Primary particle
        parentParticleType = track->GetDefinition()->GetParticleName();
    } else if (OMSimTrackingAction::HasInstance()) {
        // Secondary - look up from TrackingAction
        motherParticleName = OMSimTrackingAction::GetInstance().GetParticleType(parentTrackID);
        motherProcessName = OMSimTrackingAction::GetInstance().GetCreatorProcess(parentTrackID);
        parentParticleType = motherParticleName;

        if (motherParticleName.empty() || motherParticleName == "Unknown") {
            parentParticleType = "UnknownParticle";
        }
        if (motherProcessName.empty()) {
            motherProcessName = "UnknownProcess";
        }
    } else {
        parentParticleType = "Unknown";
        motherProcessName = "Unknown";
    }

    // Fill basic info
    info.parentID = parentTrackID;
    info.parentType = parentParticleType;
    info.eventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
    info.globalTime = track->GetGlobalTime();
    info.localTime = track->GetLocalTime();
    info.trackLength = track->GetTrackLength() / m;
    info.kineticEnergy = track->GetKineticEnergy();

    // Calculate wavelength
    static const G4double planck_eVs = 4.135667696e-15 * eV * s;
    static const G4double c_nm_s = 2.99792458e17 * nm / s;
    info.wavelength = planck_eVs * c_nm_s / info.kineticEnergy;

    info.globalPosition = track->GetPosition();
    G4ThreeVector localPos = pStep->GetPostStepPoint()
                                 ->GetTouchableHandle()
                                 ->GetHistory()
                                 ->GetTopTransform()
                                 .TransformPoint(info.globalPosition);
    info.localPosition = localPos;
    info.momentumDirection = track->GetMomentumDirection();
    info.deltaPosition = track->GetVertexPosition() - info.globalPosition;

    info.pmtNumber = 0;
    info.detectorID = std::atoi(SensitiveDetectorName);

    // PMT response
    if (m_PMTResponse) {
        info.PMTResponse = m_PMTResponse->processPhotocathodeHit(localPos.x(), localPos.y(), info.wavelength);
    } else {
        info.PMTResponse = OMSimPMTResponse::PMTPulse({0, 0, 0});
    }

    // Classify photon origin based on creator process and parent particle
    if (creatorProcessName == "Cerenkov") {
        if (parentParticleType == "mu+" || parentParticleType == "mu-") {
            info.photonOrigin = "Cerenkov from Muon";
            info.parentProcess = "muIoni";
        } else if (parentParticleType == "e+" || parentParticleType == "e-") {
            info.photonOrigin = "Cerenkov from Electron";
            info.parentProcess = "eIoni";
        } else {
            info.photonOrigin = "Cerenkov from Other";
            info.parentProcess = motherProcessName;
        }
    } else if (creatorProcessName == "eBrem") {
        info.photonOrigin = "Bremsstrahlung";
        info.parentProcess = motherProcessName;
    } else if (creatorProcessName == "Scintillation") {
        info.photonOrigin = "Scintillation";
        info.parentProcess = motherProcessName;
    } else if (creatorProcessName == "Primary") {
        if (track->GetDefinition() == G4OpticalPhoton::Definition()) {
            info.photonOrigin = "PrimaryOpticalPhoton";
        } else {
            info.photonOrigin = "Primary";
        }
        info.parentProcess = "Primary";
    } else {
        info.photonOrigin = "Other";
        info.parentProcess = motherProcessName;
    }

    // Record entry time
    if (pStep->GetPreStepPoint()->GetStepStatus() == fGeomBoundary) {
        info.entryTime = pStep->GetPostStepPoint()->GetGlobalTime();
    } else {
        info.entryTime = info.globalTime;
    }

    return info;
}

G4bool OMSimSensitiveDetector::checkVolumeAbsorption(G4Step* pStep)
{
    return pStep->GetPostStepPoint()->GetProcessDefinedStep()->GetProcessName() == "OpAbsorption" ||
           pStep->GetTrack()->GetDefinition() != G4OpticalPhoton::Definition();
}

G4bool OMSimSensitiveDetector::checkBoundaryAbsorption(G4Step* pStep)
{
    if (m_boundaryProcess == nullptr)
        fetchBoundaryProcess();

    if (pStep->GetPostStepPoint()->GetStepStatus() == fGeomBoundary) {
        if (m_boundaryProcess) {
            G4OpBoundaryProcessStatus boundaryStatus = m_boundaryProcess->GetStatus();
            if (boundaryStatus == G4OpBoundaryProcessStatus::Detection) {
                return true;
            }
        }
        if (pStep->GetTrack()->GetDefinition() != G4OpticalPhoton::Definition()) {
            return true;
        }
    }
    return false;
}

bool OMSimSensitiveDetector::isPhotonDetected(double p_efficiency)
{
    return G4UniformRand() < p_efficiency;
}

G4bool OMSimSensitiveDetector::handlePMT(G4Step* pStep, G4TouchableHistory* pTouchableHistory)
{
    PhotonInfo info = getPhotonInfo(pStep);

    if (m_QEcut && !isPhotonDetected(info.PMTResponse.detectionProbability))
        return false;
    else if (m_QEcut)
        info.PMTResponse.detectionProbability = 1;

    G4TouchableHandle touchable = pStep->GetPreStepPoint()->GetTouchableHandle();
    G4String name;
    int i = 0;
    do {
        name = touchable->GetVolume(i)->GetName();
        i++;
    } while (name.substr(0, 3) != "PMT");

    std::vector<G4String> numberPMTstring = Tools::splitStringByDelimiter(name, '_');
    info.pmtNumber = atoi(numberPMTstring.at(1));
    storePhotonHit(info);
    killParticle(pStep->GetTrack());
    return true;
}

G4bool OMSimSensitiveDetector::handleGeneralPhotonDetector(G4Step* pStep, G4TouchableHistory* pTouchableHistory)
{
    PhotonInfo info = getPhotonInfo(pStep);
    info.pmtNumber = 0;
    storePhotonHit(info);
    killParticle(pStep->GetTrack());
    return true;
}

G4bool OMSimSensitiveDetector::handleShellDetector(G4Step* pStep, G4TouchableHistory* pTouchableHistory)
{
    PhotonInfo info = getPhotonInfo(pStep);
    info.pmtNumber = 0;
    storePhotonHit(info);
    return true;  // Don't kill particle for shell detector
}

void OMSimSensitiveDetector::storePhotonHit(PhotonInfo& pInfo)
{
    OMSimHitManager& hitManager = OMSimHitManager::getInstance();
    hitManager.appendHitInfo(
        pInfo.eventID,
        pInfo.globalTime,
        pInfo.entryTime,
        pInfo.localTime,
        pInfo.trackLength,
        pInfo.kineticEnergy,
        pInfo.pmtNumber,
        pInfo.momentumDirection,
        pInfo.globalPosition,
        pInfo.localPosition,
        pInfo.deltaPosition.mag(),
        pInfo.PMTResponse,
        pInfo.photonOrigin,
        pInfo.parentID,
        pInfo.parentType,
        pInfo.parentProcess,
        pInfo.wavelength,
        pInfo.detectorID
    );
}

void OMSimSensitiveDetector::killParticle(G4Track* pTrack)
{
    if (pTrack->GetTrackStatus() != fStopAndKill) {
        pTrack->SetTrackStatus(fStopAndKill);
    }
}
