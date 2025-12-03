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

OMSimDecaysAnalysis::OMSimDecaysAnalysis()
 : outputSufix(OMSimCommandArgsTable::getInstance().get<std::string>("output_file")),
   LightOutput(OMSimCommandArgsTable::getInstance().get<bool>("LightOutput"))
{
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
	if (LightOutput)
		return;
	if (!m_threadDecayStats)
		return;
	log_trace("Writing decay information of {} decays", m_threadDecayStats->eventId.size());

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
	if (!lHitManager.areThereHitsInModuleSingleThread())
		return;

	HitStats lHits = lHitManager.getSingleThreadHitsOfModule();
	G4String lHitsFileName = outputSufix + "_" + Tools::getThreadIDStr() + "_hits.dat";
	log_trace("Writing hit information of {} hits", lHits.eventId.size());

	std::fstream dataFile;
	dataFile.open(lHitsFileName.c_str(), std::ios::out | std::ios::app);
	if (lHits.eventId.size() == 0)
		return;
	if (LightOutput)
	{
		for (int i = 0; i < (int)lHits.eventId.size(); i++)
		{
			dataFile << lHits.eventId.at(i) << "\t";
			dataFile << std::setprecision(13);
			dataFile << lHits.hitTime.at(i) / s << "\t";
			dataFile << std::setprecision(4);
			dataFile << lHits.PMTnr.at(i) << "\t";
			dataFile << G4endl;
		}
	}
	else
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

void OMSimDecaysAnalysis::mergeThreadFiles(G4String p_fileEnd)
{
	G4String mergedFileName = outputSufix + p_fileEnd;

	std::ofstream mergedFile;
	mergedFile.open(mergedFileName.c_str(), std::ios::out | std::ios::app);
	if (!mergedFile.is_open())
	{
		log_error("Failed to open merged decay file: {}", mergedFileName);
		return;
	}

	int numThreads = OMSimCommandArgsTable::getInstance().get<int>("threads");

	for (int threadID = 0; threadID < numThreads; ++threadID)
	{
		G4String threadFileName = outputSufix + "_" + std::to_string(threadID) + p_fileEnd;

		std::ifstream threadFile(threadFileName.c_str(), std::ios::in);
		if (!threadFile.is_open())
		{
			log_warning("Failed to open thread file: {}", threadFileName);
			continue;
		}

		G4String line;
		while (std::getline(threadFile, line))
		{
			mergedFile << line << G4endl;
		}

		threadFile.close();

		// Delete the processed thread file
		if (std::remove(threadFileName.c_str()) != 0)
		{
			log_warning("Failed to delete thread file: {}", threadFileName);
		}
		else
		{
			log_trace("Deleted thread file: {}", threadFileName);
		}
	}

	mergedFile.close();
	log_trace("Merged all thread files into: {}", mergedFileName);
}

void OMSimDecaysAnalysis::mergeFiles()
{
	mergeThreadFiles(G4String("_hits.dat"));
	if (!LightOutput)
	{
		mergeThreadFiles(G4String("_decays.dat"));
	}
}