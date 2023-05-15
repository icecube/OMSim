#include "OMSimAnalysisManager.hh"
#include "G4ios.hh"
//since Geant4.10: include units manually
#include "G4SystemOfUnits.hh"

extern G4int gDOM;

OMSimAnalysisManager::OMSimAnalysisManager(){}

OMSimAnalysisManager::~OMSimAnalysisManager(){}

void OMSimAnalysisManager::Write()
{	
	for (int i = 0; i < (int) stats_event_id.size(); i++) {
		datafile << stats_event_id.at(i) << "\t";
		//datafile << stats_hit_time.at(i) << "\t";
		datafile << stats_photon_flight_time.at(i) << "\t";
		//datafile << stats_photon_track_length.at(i) << "\t";
		datafile << stats_photon_energy.at(i) << "\t";
		//datafile << stats_PMT_hit.at(i) << "\t";
		//datafile << stats_event_distance.at(i) << "\t";
		datafile << lPulses.at(i).DetectionProbability << "\t";
		datafile << lPulses.at(i).TransitTime << "\t";
		datafile << lPulses.at(i).PE << "\t";
		datafile << stats_photon_position.at(i).x()/m << "\t";
		datafile << stats_photon_position.at(i).y()/m << "\t";
		datafile << stats_photon_position.at(i).z()/m << "\t";
		//datafile << stats_photon_direction.at(i).x() << "\t";
		//datafile << stats_photon_direction.at(i).y() << "\t";
		//datafile << stats_photon_direction.at(i).z() << "\t";
		//datafile << stats_photon_position.at(i).mag() / m ;
		datafile << G4endl;
	}
	
	
}

void OMSimAnalysisManager::WriteAccept()
{
	int num_pmts;
	if (gDOM==0){num_pmts = 1;} //single PMT
	else if (gDOM==1){num_pmts = 24;} //mDOM
	else if (gDOM==2){num_pmts = 1;} //PDOM
	else if (gDOM==3){num_pmts = 16;} //LOM16
	else if (gDOM==4){num_pmts = 18;} //LOM18
	else if (gDOM==5){num_pmts = 2;} //DEGG
    else{num_pmts = 99;} //custom

	

		
	double	pmthits[num_pmts+1] = {0};
	double sum = 0;
	
	// repacking hits:
	for (int i = 0; i < (int) stats_PMT_hit.size(); i++) {

		pmthits[stats_PMT_hit.at(i)] += 1;//lPulses.at(i).DetectionProbability;
	}
	// wrinting collective hits
	for (int j = 0; j < num_pmts; j++) {
		datafile << "\t" << pmthits[j];
		sum += pmthits[j];
		pmthits[j] = 0;
	}
	datafile << "\t" << sum;
	datafile << G4endl;	
}


void OMSimAnalysisManager::Reset()
{
	stats_event_id.clear();
	stats_photon_flight_time.clear();
	stats_photon_track_length.clear();
	stats_hit_time.clear();
	stats_photon_energy.clear();
	stats_PMT_hit.clear();
	stats_photon_direction.clear();
	stats_photon_position.clear();
    stats_event_distance.clear();
	lPulses.clear();
}

