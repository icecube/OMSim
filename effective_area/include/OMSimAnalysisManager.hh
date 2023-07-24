#ifndef OMSimAnalysisManager_h
#define OMSimAnalysisManager_h 1

#include "G4Types.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"
#include "OMSimPMTResponse.hh"
#include <vector>
#include <fstream>

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

class OMSimAnalysisManager
{
public:
    static OMSimAnalysisManager &getInstance()
    {
        static OMSimAnalysisManager instance;
        return instance;
    }

    std::vector<double> CountHits();
    void Reset();
	void WriteScan(G4double pPhi, G4double pTheta);
	void WriteHeader();

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
