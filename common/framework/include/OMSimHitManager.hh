/**
 * @file OMSimHitManager.hh
 * @brief Defines structures and classes related to optical module photon hit management.
 *
 * This file describes the `HitStats` structure which stores detailed photon hit parameters
 * and the `OMSimHitManager` class which centralizes and manages photon hit data across
 * different optical modules. The manager ensures unified access to photon hit information,
 * with functionalities to append, retrieve, and manipulate hit data.
 *
 * @ingroup common
 */

#ifndef OMSimHitManager_h
#define OMSimHitManager_h 1

#include "OMSimPMTResponse.hh"

#include <G4ThreeVector.hh>
#include <fstream>
#include <G4AutoLock.hh>
#include <G4Threading.hh>

/**
 * @struct HitStats
 * @brief A structure of vectors to store information about detected photons.
 * @ingroup common
 */
struct HitStats
{
    std::vector<G4long> eventId;                         ///< ID of the event
    std::vector<G4double> hitTime;                       ///< Time of detection.
    std::vector<G4double> flightTime;                    ///< Photon flight time.
    std::vector<G4double> pathLenght;                    ///< Length of the photon's path before hitting.
    std::vector<G4double> energy;                        ///< Energy of the detected photon.
    std::vector<G4int> PMTnr;                            ///< ID of the PMT that detected the photon.
    std::vector<G4ThreeVector> direction;                ///< Momentum direction of the photon at the time of detection.
    std::vector<G4ThreeVector> localPosition;            ///< Local position of the detected photon within the PMT.
    std::vector<G4ThreeVector> globalPosition;           ///< Global position of the detected photon.
    std::vector<G4double> generationDetectionDistance;   ///< Distance between generation and detection of photon.
    std::vector<OMSimPMTResponse::PMTPulse> PMTresponse; ///< PMT's response to the detected photon, encapsulated as a `PMTPulse`.
};

/**
 * @class OMSimHitManager
 * @brief Manages detected photon information.
 *
 * Stores, manages, and provides access information related to detected photons across multiple optical modules.
 * The manager uses a global instance pattern, ensuring a unified access point for photon hit data.
 *
 * The class must be initialized with `OMSimHitManager::init()` before use and shut down with 
 * `OMSimHitManager::shutdown()` when no longer needed. This is handled by the OMSim class.
 *
 * The hits are stored using 'OMSimHitManager::appendHitInfo'.
 * The analysis manager of each study is in charge of writing the stored information into a file (see for example 'OMSimEffectiveAreaAnalyisis::writeScan' or 'OMSimDecaysAnalysis::writeHitInformation').
 * If the simulation will continue after the data is written, do not forget calling 'OMSimHitManager::reset'!.
 *
 * @ingroup common
 */
class OMSimHitManager
{
    OMSimHitManager();
    ~OMSimHitManager() = default;
    OMSimHitManager(const OMSimHitManager &) = delete;
    OMSimHitManager &operator=(const OMSimHitManager &) = delete;

public:
    static void init();
    static void shutdown();
    static OMSimHitManager &getInstance();

    void appendHitInfo(
        G4double pGlobalTime,
        G4double pLocalTime,
        G4double pTrackLength,
        G4double pEnergy,
        G4int pPMTHitNumber,
        G4ThreeVector pMomentumDirection,
        G4ThreeVector pGlobalPos,
        G4ThreeVector pLocalPos,
        G4double pDistance,
        OMSimPMTResponse::PMTPulse pResponse,
        G4int pModuleIndex = 0);

    void reset();
    std::vector<double> countMergedHits(int pModuleIndex = 0);
    void setNumberOfPMTs(int pNumberOfPMTs, int pModuleIndex = 0);
    HitStats getMergedHitsOfModule(int pModuleIndex = 0);
    HitStats getSingleThreadHitsOfModule(int pModuleIndex = 0);
    bool areThereHitsInModuleSingleThread(int pModuleIndex = 0);
    void sortHitStatsByTime(HitStats &pHits);
    std::vector<int> calculateMultiplicity(const G4double pTimeWindow, int pModuleNumber = 0);
    G4int getNextDetectorIndex() { return ++mCurrentIndex; }
    G4int getNumberOfModules() { return mCurrentIndex + 1; }

    void mergeThreadData();

    std::map<G4int, HitStats> mModuleHits; ///< Map of a HitStats containing hit information for each simulated optical module

private:
    std::map<G4int, G4int> mNumPMTs; ///< Map of number of PMTs in the used optical modules
    G4int mCurrentIndex;

    static G4Mutex mMutex;
    static OMSimHitManager *mInstance;

    struct ThreadLocalData
    {
        std::map<G4int, HitStats> moduleHits;
    };
    G4ThreadLocal static ThreadLocalData *mThreadData;
};

inline OMSimHitManager *gHitManager = nullptr;

#endif
