#ifndef OMSimHitManager_h
#define OMSimHitManager_h 1

#include "OMSimPMTResponse.hh"

#include <G4ThreeVector.hh>
#include <fstream>

/**
 * @struct HitStats
 * @brief A structure to store information of photon hits.
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
 * @brief This class is responsible for managing info of detected photons and writing the results in the desired format.
 * @ingroup common
 */
class OMSimHitManager
{

public:

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
    void setNumberOfPMTs(int pNumberOfPMTs);
    HitStats getHitsOfModule(int moduleIndex=0);

    private:
        int mNumPMTs;
        std::unordered_map<G4int, HitStats> mModuleHits;
        OMSimHitManager() = default;
        ~OMSimHitManager() = default;
        OMSimHitManager(const OMSimHitManager &) = delete;
        OMSimHitManager &operator=(const OMSimHitManager &) = delete;
};

#endif
