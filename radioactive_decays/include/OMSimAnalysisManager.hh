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
 * @class OMSimAnalysisManager
 * @brief This class is responsible for managing info of detected photons and writing the results in the desired format.
 * @ingroup EffectiveArea
 */
class OMSimAnalysisManager
{
public:
    static OMSimAnalysisManager &getInstance()
    {
        static OMSimAnalysisManager instance;
        return instance;
    }

    std::vector<double> countHits();
    void reset();
	void writeScan(G4double pPhi, G4double pTheta);
	void writeHeader();
    void appendDecay(G4String pParticleName, G4double pTime,  G4ThreeVector pPositionVector){};

    G4String mOutputFileName;
    std::fstream mDatafile;
    G4long mCurrentEventNumber;
    HitStats mHits;

private:
    OMSimAnalysisManager() = default;
    ~OMSimAnalysisManager() = default;
    OMSimAnalysisManager(const OMSimAnalysisManager &) = delete;
    OMSimAnalysisManager &operator=(const OMSimAnalysisManager &) = delete;
};

#endif
