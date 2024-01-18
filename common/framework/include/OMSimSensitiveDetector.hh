#ifndef OMSimSensitiveDetector_h
#define OMSimSensitiveDetector_h 1

#include "G4VSensitiveDetector.hh"
#include "G4ThreeVector.hh"
#include "OMSimPMTResponse.hh"
#include "G4OpBoundaryProcess.hh"
#include <vector>

class G4Step;
class G4TouchableHistory;


/**
 * @enum DetectorType
 * @brief Enumerates the types of detectors supported by OMSimSensitiveDetector.
 *  Additional types can be added as needed.
 */
enum class DetectorType {
    PMT,                           ///< Photomultiplier tube detector.
    VolumePhotonDetector,          ///< Photon detector based on absorption in volume.
    BoundaryPhotonDetector         ///< Photon detector based on absorption in boundary.
};

/**
 * @struct PhotonInfo
 * @brief Contains information about a detected photon which will be appended in HitManager.
 */
struct PhotonInfo {
    G4double globalTime;            ///< Global time of the photon hit.
    G4double localTime;             ///< Local time of the photon hit.
    G4double trackLength;           ///< Length of the photon's track.
    G4double kineticEnergy;         ///< Kinetic energy of the photon.
    G4double wavelength;            ///< Wavelength of the photon.
    G4ThreeVector globalPosition;   ///< Global position of the photon hit.
    G4ThreeVector localPosition;    ///< Local position of the photon hit.
    G4ThreeVector momentumDirection;///< Direction of the photon's momentum.
    G4ThreeVector deltaPosition;    ///< Change in position of the photon hit.
    G4int pmtNumber;                ///< PMT number associated with the photon hit.
    G4int detectorID;               ///< ID of the detector registering the photon.
    OMSimPMTResponse::PMTPulse PMTResponse; ///< PMT response to the photon hit.
};



/**
 * @class OMSimSensitiveDetector
 * @brief Represents a sensitive detector.
 */
class OMSimSensitiveDetector : public G4VSensitiveDetector
{
public:
    OMSimSensitiveDetector(G4String name, DetectorType pDetectorType);
    virtual ~OMSimSensitiveDetector() {};

    G4bool ProcessHits(G4Step *pStep, G4TouchableHistory *pTouchableHistory) override;
    void setPMTResponse(OMSimPMTResponse *pResponse);

private:
    OMSimPMTResponse *mPMTResponse;
    DetectorType mDetectorType;
    G4OpBoundaryProcess* mBoundaryProcess = nullptr;

    G4bool checkVolumeAbsorption(G4Step *pStep);
    G4bool checkBoundaryAbsorption(G4Step *pStep);
    PhotonInfo getPhotonInfo(G4Step *pStep);
    G4bool handlePMT(G4Step *pStep, G4TouchableHistory *pTouchableHistory);
    G4bool handleGeneralPhotonDetector(G4Step *pStep, G4TouchableHistory *pTouchableHistory);
    void storePhotonHit(PhotonInfo &info);
    void fetchBoundaryProcess();
};

#endif