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

/**
 * @struct HitStats
 * @brief A structure to store information about detected photon hits.
 *
 * This structure holds a set of vectors that keep track of the various parameters
 * related to photon hits detected by the optical modules and PMTs.
 * @ingroup common
 */
struct HitStats
{
    std::vector<G4long> event_id;
    std::vector<G4double> hit_time; //Time of detection.
    std::vector<G4double> photon_flight_time; //Photon flight time.
    std::vector<G4double> photon_track_length; //Length of the photon's path before hitting.
    std::vector<G4double> photon_energy; //Energy of the detected photon.
    std::vector<G4int> PMT_hit; //ID of the PMT that detected the photon.
    std::vector<G4ThreeVector> photon_direction; //Momentum direction of the photon at the time of detection.
    std::vector<G4ThreeVector> photon_local_position; //Local position of the detected photon within the PMT.
    std::vector<G4ThreeVector> photon_global_position; //Global position of the detected photon.
    std::vector<G4double> event_distance; //Distance between generation and detection of photon.
    std::vector<OMSimPMTResponse::PMTPulse> PMT_response; // PMT's response to the detected photon, encapsulated as a `PMTPulse`.
};

/**
 * @class OMSimHitManager
 * @brief Manages detected photon information.
 *
 * This class is responsible for storing, managing, and providing access to
 * the hit information related to detected photons across multiple optical modules.
 * The manager can be accessed via a singleton pattern, ensuring a unified access
 * point for photon hit data.
 *
 * @ingroup common
 */
class OMSimHitManager
{

public:
    /**
     * @brief Returns the singleton instance of the OMSimHitManager.
     * @return A reference to the singleton instance.
     */
    static OMSimHitManager &getInstance()
    {
        static OMSimHitManager instance;
        return instance;
    }

    /**
     * @brief Appends hit information for a detected photon to the corresponding module's hit data.
     *
     * This method appends hit information to the corresponding module's `HitStats` structure in the manager.
     * If the specified module number is not yet in the manager, a new `HitStats` structure is created for it.
     *
     * @param globalTime Time of detection.
     * @param localTime Photon flight time.
     * @param trackLength Length of the photon's path before hitting.
     * @param energy Energy of the detected photon.
     * @param PMTHitNumber ID of the PMT that detected the photon.
     * @param momentumDirection Momentum direction of the photon at the time of detection.
     * @param globalPos Global position of the detected photon.
     * @param localPos Local position of the detected photon within the PMT.
     * @param distance Distance between generation and detection of photon.
     * @param response PMT's response to the detected photon, encapsulated as a `PMTPulse`.
     * @param moduleNumber ID of the module in which the photon was detected.
     */
    void appendHitInfo(
        G4double globalTime,
        G4double localTime,
        G4double trackLength,
        G4double energy,
        G4int PMTHitNumber,
        G4ThreeVector momentumDirection,
        G4ThreeVector globalPos,
        G4ThreeVector localPos,
        G4double distance,
        OMSimPMTResponse::PMTPulse response,
        G4int moduleIndex = 0);

    /**
     * @brief Resets hit information for all modules.
     */
    void reset();

    /**
     * @brief Counts hits for a specified module.
     * @param moduleIndex Index of the module for which to count hits. Default is 0.
     * @return A vector containing the hit count for each PMT in the specified module.
     */
    std::vector<double> countHits(int moduleIndex = 0);

    /**
     * @brief Saves the number of PMTs in a module
     * @param pNumberOfPMTs Nr of PMTs in OM
     * @param pModuleIndex Module index for which we are getting the information (default 0)
     */
    void setNumberOfPMTs(int pNumberOfPMTs, int moduleIndex = 0);

    /**
     * @brief Retrieves the hit statistics for a specified module.
     * @param moduleIndex Index of the module for which to retrieve hit statistics. Default is 0.
     * @return A HitStats structure containing hit information for the specified module.
     */
    HitStats getHitsOfModule(int moduleIndex = 0);

    /**
     * @brief Sorts the hit statistics by the hit time.
     * @param lHits The hit statistics to be sorted.
     */
    void sortHitStatsByTime(HitStats &lHits);

    /**
     * @brief Calculates the multiplicity of hits within a specified time window for a given module.
     *
     * This method determines the multiplicity of hits for the specified optical module within a
     * given time window. Multiplicity is defined as the number of PMTs detecting a hit within the
     * time window. The result is a vector where each element represents the number of occurrences
     * of a specific multiplicity.
     *
     * For instance, if the resulting vector is [5, 3, 2], it means:
     * - 5 occurrences of 1 PMT detecting a hit within the time window.
     * - 3 occurrences of 2 PMTs detecting hits within the same window.
     * - 2 occurrences of 3 PMTs detecting hits within the window.
     *
     * @param pTimeWindow The time window within which to calculate the multiplicity (in seconds).
     * @param moduleNumber The index of the module for which to calculate the multiplicity. Default is 0.
     * @return A vector containing the multiplicity data.
     */
    std::vector<int> calculateMultiplicity(const G4double pTimeWindow, int moduleNumber = 0);

private:
    std::map<G4int, G4int> mNumPMTs;
    std::map<G4int, HitStats> mModuleHits;
    OMSimHitManager() = default;
    ~OMSimHitManager() = default;
    OMSimHitManager(const OMSimHitManager &) = delete;
    OMSimHitManager &operator=(const OMSimHitManager &) = delete;
};

#endif
