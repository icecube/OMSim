#ifndef mdomAnalysisManager_h
#define mdomAnalysisManager_h 1

#include "G4Types.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"

#include <vector>
#include <fstream>
#include <tuple>

struct HitStat {
    HitStat() {};
    ~HitStat() {};
	G4int moduleNr;
	G4int pmtNr;
	G4double theta;
	G4double phi;
	G4double hit_time;
	G4double wavelen;
	G4ThreeVector position;
	G4double QEprob;
};

struct EvtStat {
    EvtStat() {};
    ~EvtStat() {};
  std::vector<G4int> modulescounter; // helper parameter to get how many modules where hit
  G4int nrHitTot;// Total number of hits
  G4int nrHitPMTs;// Number of PMTs hit
  G4int nrHitMod; //Number of modules hit
  std::vector<std::tuple<G4int,G4int,G4int> > hitsPMTs;// Module Nr | PMT number | Hits in this PMT
};



class MdomAnalysisManager
{
	public:
		MdomAnalysisManager();
		~MdomAnalysisManager();
		void ResetEvent();
		void AnalyzeEvent();
        void Helper_AnalyzeEvent(EvtStat& this_evtStat);
		void WriteHeader();
        void HelpTheHeader(std::fstream& thisfile);
        void Writer_InfoFile();
        void Writer_data(std::fstream& thisfile, EvtStat& this_evtStat);
        void Helper_ResetEvent(EvtStat& this_evtStat);

		G4double nuTime;
		G4double nuMeanEnergy;
		G4double nuEnergy;
		G4double cosTheta;
		G4double weigh;
		G4double primaryEnergy;
		// event quantities
		G4bool foundPhoton;
		G4double primaryX;
		G4double primaryY;
		G4double primaryZ;
		G4double primaryR;
		G4double primaryDirX;
		G4double primaryDirY;
		G4double primaryDirZ;
		G4double photonTheta;
		G4double photonPhi;
		G4double photonR;
		std::vector<HitStat> hitStats;
		G4double TotHits[24];
		// run quantities
		G4String outputFilename;
		std::fstream datafile;
        std::fstream maininfofile;
        std::fstream datafile_gammas2MeV;
        std::fstream datafile_gammas8MeV;
		G4double realdistance;
        
        EvtStat evtStat0; 
	private:
	 //there is no privacy here
};

#endif
