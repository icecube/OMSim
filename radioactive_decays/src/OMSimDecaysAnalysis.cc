#include "OMSimDecaysAnalysis.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimEventAction.hh"
#include "OMSimHitManager.hh"
#include <numeric>


void OMSimDecaysAnalysis::appendDecay(G4String pParticleName, G4double pDecayTime, G4ThreeVector pDecayPosition)
{
	G4int lEventID = EventInfoManager::getInstance().getCurrentEventID();
	mDecaysStats.event_id.push_back(lEventID);
	mDecaysStats.isotope_name.push_back(pParticleName);
	mDecaysStats.decay_time.push_back(pDecayTime);
	mDecaysStats.decay_position.push_back(pDecayPosition);
}

void OMSimDecaysAnalysis::setOutputFileName(G4String pName)
{
	mMultiplicityFileName = pName+"_multiplicity.dat";
	mHitsFileName = pName+"_hits.dat";
	mDecaysFileName = pName+"_decays.dat";
}

void OMSimDecaysAnalysis::writeMultiplicity()
{ 
	std::vector<int>  lMultiplicity = OMSimHitManager::getInstance().calculateMultiplicity(20*ns);
	mDatafile.open(mMultiplicityFileName.c_str(), std::ios::out | std::ios::app);
	for (const auto &value : lMultiplicity)
	{
		mDatafile << value << "\t";
	}
	mDatafile << G4endl;
	mDatafile.close();
}



void OMSimDecaysAnalysis::writeDecayInformation()
{
	mDatafile.open(mDecaysFileName.c_str(), std::ios::out | std::ios::app);

	for (int i = 0; i < (int)mDecaysStats.event_id.size(); i++)
	{
		mDatafile << mDecaysStats.event_id.at(i) << "\t";
		mDatafile << mDecaysStats.decay_time.at(i) << "\t";
		mDatafile << mDecaysStats.isotope_name.at(i) << "\t";
		mDatafile << mDecaysStats.decay_position.at(i).x() << "\t";
		mDatafile << mDecaysStats.decay_position.at(i).y() << "\t";
		mDatafile << mDecaysStats.decay_position.at(i).z() << "\t";
	}
	mDatafile << G4endl;

	mDatafile.close();
}

void OMSimDecaysAnalysis::writeHitInformation()
{
	HitStats lHits = OMSimHitManager::getInstance().getHitsOfModule();

	mDatafile.open(mHitsFileName.c_str(), std::ios::out | std::ios::app);

	for (int i = 0; i < (int)lHits.event_id.size(); i++)
	{
		mDatafile << lHits.event_id.at(i) << "\t";
		mDatafile << lHits.hit_time.at(i) << "\t";
		mDatafile << lHits.PMT_hit.at(i) << "\t";
		mDatafile << lHits.photon_energy.at(i) << "\t";
		mDatafile << lHits.photon_global_position.at(i).x() << "\t";
		mDatafile << lHits.photon_global_position.at(i).y() << "\t";
		mDatafile << lHits.photon_global_position.at(i).z() << "\t";
		mDatafile << lHits.PMT_response.at(i).PE << "\t";
		mDatafile << lHits.PMT_response.at(i).TransitTime << "\t";
		mDatafile << lHits.PMT_response.at(i).DetectionProbability << "\t";

	}
	mDatafile << G4endl;
	mDatafile.close();
}
