/**
 * @file OMSimSensitiveDetector.hh
 * @brief Enhanced sensitive detector for WavePID with photon origin tracking.
 */
#pragma once

#include "G4VSensitiveDetector.hh"
#include "G4ThreeVector.hh"
#include "OMSimPMTResponse.hh"

#include <vector>
#include <string>

class G4Step;
class G4Track;
class G4TouchableHistory;
class G4OpBoundaryProcess;

/**
 * @enum DetectorType
 * @brief Types of detectors supported by OMSimSensitiveDetector.
 */
enum class DetectorType {
    PMT,
    VolumePhotonDetector,
    BoundaryPhotonDetector,
    BoundaryShellDetector,
    PerfectPMT
};

/**
 * @struct PhotonInfo
 * @brief Extended photon information with origin tracking for WavePID.
 */
struct PhotonInfo {
    G4int eventID;
    G4double globalTime;
    G4double localTime;
    G4double trackLength;
    G4double kineticEnergy;
    G4double wavelength;
    G4ThreeVector globalPosition;
    G4ThreeVector localPosition;
    G4ThreeVector momentumDirection;
    G4ThreeVector deltaPosition;
    G4int pmtNumber;
    G4int detectorID;
    OMSimPMTResponse::PMTPulse PMTResponse;
    // WavePID extensions for photon origin tracking
    G4String photonOrigin;      ///< Origin classification (e.g., "Cerenkov from Muon")
    G4int parentID;             ///< Parent track ID
    std::string parentType;     ///< Parent particle type (e.g., "mu-", "e-")
    G4String parentProcess;     ///< Process that created parent particle
    G4double entryTime;         ///< Time photon entered the DOM
};

/**
 * @class OMSimSensitiveDetector
 * @brief Enhanced sensitive detector with photon origin classification.
 */
class OMSimSensitiveDetector : public G4VSensitiveDetector
{
public:
    OMSimSensitiveDetector(G4String pName, DetectorType pDetectorType);
    ~OMSimSensitiveDetector() override;

    G4bool ProcessHits(G4Step* pStep, G4TouchableHistory* pTouchableHistory) override;
    void setPMTResponse(OMSimPMTResponse* pResponse);

private:
    bool m_QEcut;
    OMSimPMTResponse* m_PMTResponse;
    DetectorType m_detectorType;

    static thread_local G4OpBoundaryProcess* m_boundaryProcess;

    G4bool checkVolumeAbsorption(G4Step* pStep);
    G4bool checkBoundaryAbsorption(G4Step* pStep);
    PhotonInfo getPhotonInfo(G4Step* pStep);
    G4bool handlePMT(G4Step* pStep, G4TouchableHistory* pTouchableHistory);
    G4bool handleGeneralPhotonDetector(G4Step* pStep, G4TouchableHistory* pTouchableHistory);
    G4bool handleShellDetector(G4Step* pStep, G4TouchableHistory* pTouchableHistory);
    bool isPhotonDetected(double p_efficiency);
    void storePhotonHit(PhotonInfo& pInfo);
    void fetchBoundaryProcess();
    void killParticle(G4Track* pTrack);
};
