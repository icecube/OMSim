#ifndef OMSimAnalysisManager_h
#define OMSimAnalysisManager_h 1

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
 * @struct DecayStats
 * @brief A structure to store information of the decays.
 */
struct DecayStats
{
    std::vector<G4long> event_id;
    std::vector<G4double> isotope_name;
    std::vector<G4double> decay_time;
    std::vector<G4ThreeVector> decay_position;
};

/** 
 * @class OMSimHitManager
 * @brief This class is responsible for managing info of detected photons and writing the results in the desired format.
 * @ingroup EffectiveArea
 */
class OMSimHitManager
{
public:
    static OMSimHitManager &getInstance()
    {
        static OMSimHitManager instance;
        return instance;
    }

    std::vector<double> countHits();
    void reset();
	void writeScan(G4double pPhi, G4double pTheta);
	void writeHeader();
    void appendDecay(G4String pParticleName, G4double pTime,  G4ThreeVector pPositionVector){};
    std::vector<int> OMSimHitManager::calculateMultiplicity(const G4double timeWindow, std::size_t PMT_COUNT);
    void writeMultiplicity(const std::vector<int> &pMultiplicity);
    void writeDecayData();

    G4String mOutputFileName;
    std::fstream mDatafile;
    G4long mCurrentEventNumber;

private:
    OMSimHitManager() = default;
    ~OMSimHitManager() = default;
    OMSimHitManager(const OMSimHitManager &) = delete;
    void sortHitStatsByTime(HitStats& hits);

    template <typename T>
    void applyPermutation(std::vector<T>& vec, const std::vector<std::size_t>& p); 

    OMSimHitManager &operator=(const OMSimHitManager &) = delete;

    DecayStats mDecays;
    HitStats mHits;
};

#endif
