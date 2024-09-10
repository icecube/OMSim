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
G4ThreadLocal DecayStats *OMSimDecaysAnalysis::m_threadDecayStats = nullptr;

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
 * @param p_particleName Name of the particle.
 * @param p_decayTime Time of the decay.
 * @param p_decayPosition Global position of the decay.
 */
void OMSimDecaysAnalysis::appendDecay(G4String p_particleName, G4double p_decayTime, G4ThreeVector p_decayPosition)
{
	if (!m_threadDecayStats)
	{
		log_debug("Initialized m_threadData for thread {}", G4Threading::G4GetThreadId());
		m_threadDecayStats = new DecayStats();
	}
	G4int lEventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
	m_threadDecayStats->eventId.push_back(lEventID);
	m_threadDecayStats->isotopeName.push_back(p_particleName);
	m_threadDecayStats->decayTime.push_back(p_decayTime);
	m_threadDecayStats->decayPosition.push_back(p_decayPosition);
}

/**
 * @brief Calls calculateMultiplicity and writes the results to the output file.
 */
void OMSimDecaysAnalysis::writeMultiplicity(G4double p_timeWindow)
{
	G4String outputSufix = OMSimCommandArgsTable::getInstance().get<std::string>("output_file");
	G4String fileName = outputSufix + "_multiplicity.dat";

	OMSimHitManager::getInstance().mergeThreadData();
	std::vector<int> multiplicity = OMSimHitManager::getInstance().calculateMultiplicity(p_timeWindow);
	
	std::fstream dataFile;
	dataFile.open(fileName.c_str(), std::ios::out | std::ios::app);

	for (const auto &value : multiplicity)
	{
		dataFile << value << "\t";
	}
	dataFile << G4endl;
	dataFile.close();
}

/**
 * @brief Write isotoped related data to the output file.
 */
void OMSimDecaysAnalysis::writeThreadDecayInformation()
{
	if (!m_threadDecayStats) return;
	log_trace("Writing decay information of {} decays", m_threadDecayStats->eventId.size());

	G4String outputSufix = OMSimCommandArgsTable::getInstance().get<std::string>("output_file");

	G4String lDecaysFileName = outputSufix + "_" + Tools::getThreadIDStr() + "_decays.dat";

	std::fstream dataFile;
	dataFile.open(lDecaysFileName.c_str(), std::ios::out | std::ios::app);
	if (m_threadDecayStats->eventId.size() > 0)
	{
		for (int i = 0; i < (int)m_threadDecayStats->eventId.size(); i++)
		{
			dataFile << m_threadDecayStats->eventId.at(i) << "\t";
			dataFile << std::setprecision(13);
			dataFile << m_threadDecayStats->decayTime.at(i) << "\t";
			dataFile << std::setprecision(4);
			dataFile << m_threadDecayStats->isotopeName.at(i) << "\t";
			dataFile << m_threadDecayStats->decayPosition.at(i).x() << "\t";
			dataFile << m_threadDecayStats->decayPosition.at(i).y() << "\t";
			dataFile << m_threadDecayStats->decayPosition.at(i).z() << "\t";
			dataFile << G4endl;
		}
	}
	dataFile.close();
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
	G4String outputSufix = OMSimCommandArgsTable::getInstance().get<std::string>("output_file");
	G4String lHitsFileName = outputSufix + "_" + Tools::getThreadIDStr() + "_hits.dat";
	log_trace("Writing hit information of {} hits", lHits.eventId.size());

	std::fstream dataFile;
	dataFile.open(lHitsFileName.c_str(), std::ios::out | std::ios::app);
	if (lHits.eventId.size() > 0)
	{
		for (int i = 0; i < (int)lHits.eventId.size(); i++)
		{
			dataFile << lHits.eventId.at(i) << "\t";
			dataFile << std::setprecision(13);
			dataFile << lHits.hitTime.at(i) / s << "\t";
			dataFile << std::setprecision(4);
			dataFile << lHits.PMTnr.at(i) << "\t";
			dataFile << lHits.energy.at(i) << "\t";
			dataFile << lHits.globalPosition.at(i).x() << "\t";
			dataFile << lHits.globalPosition.at(i).y() << "\t";
			dataFile << lHits.globalPosition.at(i).z() << "\t";
			dataFile << lHits.PMTresponse.at(i).PE << "\t";
			dataFile << lHits.PMTresponse.at(i).transitTime << "\t";
			dataFile << lHits.PMTresponse.at(i).detectionProbability << "\t";
			dataFile << G4endl;
		}
	}
	dataFile.close();
	log_trace("Finished writing detailed hit information");
}

/**
 * @brief Resets (deletes) decay and hits data.
 */
void OMSimDecaysAnalysis::reset()
{
	log_trace("Deleting m_threadDecayStats of Thread ID {}", G4Threading::G4GetThreadId());
	delete m_threadDecayStats;
	m_threadDecayStats = nullptr;
	OMSimHitManager::getInstance().reset();
}