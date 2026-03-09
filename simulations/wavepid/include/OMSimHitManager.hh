/**
 * @file OMSimHitManager.hh
 * @brief Enhanced hit manager for WavePID with photon origin tracking.
 *
 * This version extends the standard HitStats structure with additional fields
 * for photon origin classification (photonOrigin, parentID, parentType, etc.)
 * and integrates with ROOTHitManager for ROOT file output.
 */
#pragma once

#include "OMSimPMTResponse.hh"
#include "ROOTHitManager.hh"

#include <G4ThreeVector.hh>
#include <fstream>
#include <G4AutoLock.hh>
#include <G4Threading.hh>
#include <map>
#include <vector>

/**
 * @struct HitStats
 * @brief Extended structure for storing detected photon information with origin tracking.
 */
struct HitStats
{
    std::vector<G4long> eventId;                         ///< ID of the event
    std::vector<G4double> hitTime;                       ///< Time of detection
    std::vector<G4double> entryTime;                     ///< Time photon entered DOM
    std::vector<G4double> flightTime;                    ///< Photon flight time
    std::vector<G4double> pathLenght;                    ///< Length of photon path
    std::vector<G4double> energy;                        ///< Energy of detected photon
    std::vector<G4int> PMTnr;                            ///< PMT number
    std::vector<G4ThreeVector> direction;                ///< Momentum direction
    std::vector<G4ThreeVector> localPosition;            ///< Local position in PMT
    std::vector<G4ThreeVector> globalPosition;           ///< Global position
    std::vector<G4double> generationDetectionDistance;   ///< Distance from generation to detection
    std::vector<OMSimPMTResponse::PMTPulse> PMTresponse; ///< PMT response
    std::vector<G4String> photonOrigin;                  ///< Origin classification (e.g., "Cerenkov from Muon")
    std::vector<G4int> parentID;                         ///< Parent track ID
    std::vector<G4String> parentType;                    ///< Parent particle type (e.g., "mu-", "e-")
    std::vector<G4String> parentProcess;                 ///< Process that created parent
};

/**
 * @class OMSimHitManager
 * @brief Enhanced hit manager with photon origin tracking and ROOT output.
 */
class OMSimHitManager
{
    OMSimHitManager();
    ~OMSimHitManager() = default;
    OMSimHitManager(const OMSimHitManager&) = delete;
    OMSimHitManager& operator=(const OMSimHitManager&) = delete;

public:
    /**
     * @brief Initialize the global instance (called by OMSim::initialiseSimulation).
     */
    static void init();

    /**
     * @brief Set the ROOT output file (call after init, before simulation runs).
     * @param filename Path to output ROOT file.
     */
    static void setROOTOutputFile(const std::string& filename);

    /**
     * @brief Shutdown and cleanup the global instance.
     */
    static void shutdown();

    /**
     * @brief Get reference to the global instance.
     */
    static OMSimHitManager& getInstance();

    /**
     * @brief Append hit information with photon origin data.
     */
    void appendHitInfo(
        G4int p_eventid,
        G4double p_globalTime,
        G4double p_entryTime,
        G4double p_localTime,
        G4double p_trackLength,
        G4double p_energy,
        G4int pPMTHitNumber,
        G4ThreeVector pMomentumDirection,
        G4ThreeVector pGlobalPos,
        G4ThreeVector pLocalPos,
        G4double p_distance,
        OMSimPMTResponse::PMTPulse pResponse,
        G4String pPhotonOrigin,
        G4int pParentID,
        std::string pParentType,
        G4String pParentProcess,
        G4double p_wavelength,
        G4int pModuleIndex);

    void reset();
    std::vector<double> countMergedHits(int pModuleIndex = 0, bool pDEweight = false);
    void setNumberOfPMTs(int pNumberOfPMTs, int pModuleIndex = 0);
    HitStats getMergedHitsOfModule(int pModuleIndex = 0);
    HitStats getSingleThreadHitsOfModule(int pModuleIndex = 0);
    bool areThereHitsInModuleSingleThread(int pModuleIndex = 0);
    void sortHitStatsByTime(HitStats& pHits);
    std::vector<int> calculateMultiplicity(const G4double pTimeWindow, int pModuleNumber = 0);
    G4int getNextDetectorIndex() { return ++m_currentIndex; }
    G4int getNumberOfModules() { return m_currentIndex + 1; }

    void mergeThreadData();

    std::map<G4int, HitStats> m_moduleHits;

private:
    std::map<G4int, G4int> m_numberOfPMTs;
    G4int m_currentIndex;
    static G4Mutex m_mutex;
    static OMSimHitManager* m_instance;

    struct ThreadLocalData {
        std::map<G4int, HitStats> moduleHits;
    };
    G4ThreadLocal static ThreadLocalData* m_threadData;

    ROOTHitManager* m_rootManager = nullptr;
};

inline OMSimHitManager* g_hitManager = nullptr;
