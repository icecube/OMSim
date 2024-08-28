#include "OMSimDecaysAnalysis.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimEventAction.hh"
#include "OMSimHitManager.hh"
#include <numeric>
#include "G4AutoLock.hh"
#include "OMSimTools.hh"

G4Mutex OMSimDecaysAnalysis::m_mutex = G4Mutex();
OMSimDecaysAnalysis *OMSimDecaysAnalysis::m_instance = nullptr;
G4ThreadLocal DecayStats *OMSimDecaysAnalysis::mThreadDecayStats = nullptr;

OMSimDecaysAnalysis &OMSimDecaysAnalysis::getInstance()
{

	if (!m_instance)
	{
		G4AutoLock lock(&m_mutex);
		if (!m_instance)
		{
			m_instance = new OMSimDecaysAnalysis();
		}
	}

	return *m_instance;
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
		log_debug("Initialized m_threadData for thread {}", G4Threading::G4GetThreadId());
		mThreadDecayStats = new DecayStats();
	}
	G4int lEventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
	mThreadDecayStats->eventId.push_back(lEventID);
	mThreadDecayStats->isotope_name.push_back(pParticleName);
	mThreadDecayStats->decay_time.push_back(pDecayTime);
	mThreadDecayStats->decay_position.push_back(pDecayPosition);
}

/**
 * @brief Calls calculateMultiplicity and writes the results to the output file.
 */
void OMSimDecaysAnalysis::writeMultiplicity(G4double pTimeWindow)
{
	G4String lOutputSuffix = OMSimCommandArgsTable::getInstance().get<std::string>("output_file");
	G4String lMultiplicityFileName = lOutputSuffix + "_multiplicity.dat";

	OMSimHitManager::getInstance().mergeThreadData();
	std::vector<int> lMultiplicity = OMSimHitManager::getInstance().calculateMultiplicity(pTimeWindow);
	
	std::fstream lDatafile;
	lDatafile.open(lMultiplicityFileName.c_str(), std::ios::out | std::ios::app);

	for (const auto &value : lMultiplicity)
	{
		lDatafile << value << "\t";
	}
	lDatafile << G4endl;
	lDatafile.close();
}

/**
 * @brief Write isotoped related data to the output file.
 */
void OMSimDecaysAnalysis::writeThreadDecayInformation()
{
	log_trace("Writing decay information of {} decays", mThreadDecayStats->eventId.size());

	G4String lOutputSuffix = OMSimCommandArgsTable::getInstance().get<std::string>("output_file");

	G4String lDecaysFileName = lOutputSuffix + "_" + Tools::getThreadIDStr() + "_decays.dat";

	std::fstream lDatafile;
	lDatafile.open(lDecaysFileName.c_str(), std::ios::out | std::ios::app);
	if (mThreadDecayStats->eventId.size() > 0)
	{
		for (int i = 0; i < (int)mThreadDecayStats->eventId.size(); i++)
		{
			lDatafile << mThreadDecayStats->eventId.at(i) << "\t";
			lDatafile << std::setprecision(13);
			lDatafile << mThreadDecayStats->decay_time.at(i) << "\t";
			lDatafile << std::setprecision(4);
			lDatafile << mThreadDecayStats->isotope_name.at(i) << "\t";
			lDatafile << mThreadDecayStats->decay_position.at(i).x() << "\t";
			lDatafile << mThreadDecayStats->decay_position.at(i).y() << "\t";
			lDatafile << mThreadDecayStats->decay_position.at(i).z() << "\t";
			lDatafile << G4endl;
		}
	}
	lDatafile.close();
	log_trace("Finished writing decay information");
}

/**
 * @brief Write data of the hits to the output file.
 */
void OMSimDecaysAnalysis::writeThreadHitInformation()
{
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();
	if (!lHitManager.areThereHitsInModuleSingleThread()) return;

	HitStats lHits = lHitManager.getSingleThreadHitsOfModule();
	G4String lOutputSuffix = OMSimCommandArgsTable::getInstance().get<std::string>("output_file");
	G4String lHitsFileName = lOutputSuffix + "_" + Tools::getThreadIDStr() + "_hits.dat";
	log_trace("Writing hit information of {} hits", lHits.eventId.size());

	std::fstream lDatafile;
	lDatafile.open(lHitsFileName.c_str(), std::ios::out | std::ios::app);
	if (lHits.eventId.size() > 0)
	{
		for (int i = 0; i < (int)lHits.eventId.size(); i++)
		{
			lDatafile << lHits.eventId.at(i) << "\t";
			lDatafile << std::setprecision(13);
			lDatafile << lHits.hitTime.at(i) / s << "\t";
			lDatafile << std::setprecision(4);
			lDatafile << lHits.PMTnr.at(i) << "\t";
			lDatafile << lHits.energy.at(i) << "\t";
			lDatafile << lHits.globalPosition.at(i).x() << "\t";
			lDatafile << lHits.globalPosition.at(i).y() << "\t";
			lDatafile << lHits.globalPosition.at(i).z() << "\t";
			lDatafile << lHits.PMTresponse.at(i).PE << "\t";
			lDatafile << lHits.PMTresponse.at(i).transitTime << "\t";
			lDatafile << lHits.PMTresponse.at(i).detectionProbability << "\t";
			lDatafile << G4endl;
		}
	}
	lDatafile.close();
	log_trace("Finished writing detailed hit information");
}

/**
 * @brief Resets (deletes) decay and hits data.
 */
void OMSimDecaysAnalysis::reset()
{
	log_trace("Deleting mThreadDecayStats of Thread ID {}", G4Threading::G4GetThreadId());
	delete mThreadDecayStats;
	mThreadDecayStats = nullptr;
	OMSimHitManager::getInstance().reset();
}