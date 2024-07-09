#include "OMSimDecaysAnalysis.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimEventAction.hh"
#include "OMSimHitManager.hh"
#include <numeric>
#include "G4AutoLock.hh"

G4Mutex OMSimDecaysAnalysis::mMutex = G4Mutex();
OMSimDecaysAnalysis *OMSimDecaysAnalysis::mInstance = nullptr;
G4ThreadLocal DecayStats *OMSimDecaysAnalysis::mThreadDecayStats = nullptr;

OMSimDecaysAnalysis &OMSimDecaysAnalysis::getInstance()
{

	if (!mInstance)
	{
		G4AutoLock lock(&mMutex);
		if (!mInstance)
		{
			mInstance = new OMSimDecaysAnalysis();
		}
	}

	return *mInstance;
}

/**
 * @brief Append decay information to internal data structures.
 * @param pParticleName Name of the particle.
 * @param pDecayTime Time of the decay.
 * @param pDecayPosition Global position of the decay.
 */
void OMSimDecaysAnalysis::appendDecay(G4String pParticleName, G4double pDecayTime, G4ThreeVector pDecayPosition)
{
	if (!mThreadDecayStats)
	{
		log_debug("Initialized mThreadData for thread {}", G4Threading::G4GetThreadId());
		mThreadDecayStats = new DecayStats();
	}
	G4int lEventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
	mThreadDecayStats->eventId.push_back(lEventID);
	mThreadDecayStats->isotope_name.push_back(pParticleName);
	mThreadDecayStats->decay_time.push_back(pDecayTime);
	mThreadDecayStats->decay_position.push_back(pDecayPosition);
}

G4String OMSimDecaysAnalysis::getThreadIDStr()
{
	std::ostringstream oss;
	oss << G4Threading::G4GetThreadId();
	G4String threadIdStr = oss.str();
	return threadIdStr;
}

/**
 * @brief Calls calculateMultiplicity and writes the results to the output file.
 */
void OMSimDecaysAnalysis::writeMultiplicity()
{
	G4String lOutputSuffix = OMSimCommandArgsTable::getInstance().get<std::string>("output_file");
	G4String lMultiplicityFileName = lOutputSuffix + "_" + getThreadIDStr() + "_multiplicity.dat";

	std::vector<int> lMultiplicity = OMSimHitManager::getInstance().calculateMultiplicity(20 * ns);
	mDatafile.open(lMultiplicityFileName.c_str(), std::ios::out | std::ios::app);

	for (const auto &value : lMultiplicity)
	{
		mDatafile << value << "\t";
	}
	mDatafile << G4endl;
	mDatafile.close();
}

/**
 * @brief Write isotoped related data to the output file.
 */
void OMSimDecaysAnalysis::writeDecayInformation()
{
	G4String lOutputSuffix = OMSimCommandArgsTable::getInstance().get<std::string>("output_file");
	G4String lDecaysFileName = lOutputSuffix + "_" + getThreadIDStr() + "_decays.dat";

	mDatafile.open(lDecaysFileName.c_str(), std::ios::out | std::ios::app);
	if (mThreadDecayStats->eventId.size() > 0)
	{
		for (int i = 0; i < (int)mThreadDecayStats->eventId.size(); i++)
		{
			mDatafile << mThreadDecayStats->eventId.at(i) << "\t";
			mDatafile << std::setprecision(13);
			mDatafile << mThreadDecayStats->decay_time.at(i) << "\t";
			mDatafile << std::setprecision(4);
			mDatafile << mThreadDecayStats->isotope_name.at(i) << "\t";
			mDatafile << mThreadDecayStats->decay_position.at(i).x() << "\t";
			mDatafile << mThreadDecayStats->decay_position.at(i).y() << "\t";
			mDatafile << mThreadDecayStats->decay_position.at(i).z() << "\t";
			mDatafile << G4endl;
		}
	}
	mDatafile.close();
}

/**
 * @brief Write data of the hits to the output file.
 */
void OMSimDecaysAnalysis::writeHitInformation()
{
	HitStats lHits = OMSimHitManager::getInstance().getHitsOfModule();
	G4String lOutputSuffix = OMSimCommandArgsTable::getInstance().get<std::string>("output_file");
	G4String lHitsFileName = lOutputSuffix + "_" + getThreadIDStr() + "_hits.dat";

	mDatafile.open(lHitsFileName.c_str(), std::ios::out | std::ios::app);
	if (lHits.eventId.size() > 0)
	{
		for (int i = 0; i < (int)lHits.eventId.size(); i++)
		{
			mDatafile << lHits.eventId.at(i) << "\t";
			mDatafile << std::setprecision(13);
			mDatafile << lHits.hitTime.at(i) / s << "\t";
			mDatafile << std::setprecision(4);
			mDatafile << lHits.PMTnr.at(i) << "\t";
			mDatafile << lHits.energy.at(i) << "\t";
			mDatafile << lHits.globalPosition.at(i).x() << "\t";
			mDatafile << lHits.globalPosition.at(i).y() << "\t";
			mDatafile << lHits.globalPosition.at(i).z() << "\t";
			mDatafile << lHits.PMTresponse.at(i).PE << "\t";
			mDatafile << lHits.PMTresponse.at(i).transitTime << "\t";
			mDatafile << lHits.PMTresponse.at(i).detectionProbability << "\t";
			mDatafile << G4endl;
		}
	}
	mDatafile.close();
}

/**
 * @brief Resets (deletes) decay and hits data.
 */
void OMSimDecaysAnalysis::reset()
{
	log_trace("Deleting mThreadDecayStats of Thread ID {}", G4Threading::G4GetThreadId());
	delete mThreadDecayStats;
	mThreadDecayStats = nullptr;

}