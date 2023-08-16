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
    std::vector<G4double> hit_time;
    std::vector<G4double> photon_flight_time;
    std::vector<G4double> photon_track_length;
    std::vector<G4double> photon_energy;
    std::vector<G4int> PMT_hit;
    std::vector<G4ThreeVector> photon_direction;
    std::vector<G4ThreeVector> photon_local_position;
    std::vector<G4ThreeVector> photon_global_position;
    std::vector<G4double> event_distance;
    std::vector<OMSimPMTResponse::PMTPulse> PMT_response;
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
        G4int moduleIndex=0);

    void reset();
    std::vector<double> countHits(int moduleIndex=0);
    void setNumberOfPMTs(int pNumberOfPMTs, int moduleIndex=0);
    HitStats getHitsOfModule(int moduleIndex=0);

    private:
        std::map<G4int, G4int> mNumPMTs;
        std::map<G4int, HitStats> mModuleHits;
        OMSimHitManager() = default;
        ~OMSimHitManager() = default;
        OMSimHitManager(const OMSimHitManager &) = delete;
        OMSimHitManager &operator=(const OMSimHitManager &) = delete;
};

#endif
