/** @file OMSimSNAnalysis.cc
 *  @brief Analyze SN data and write merged output files
 */

#include "OMSimSNAnalysis.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include <OMSimTools.hh>
#include <G4AutoLock.hh>

G4ThreadLocal SNEventStats *OMSimSNAnalysis::m_eventStat = nullptr;
G4ThreadLocal bool OMSimSNAnalysis::m_headerWasWritten = false;

// NEW: static members for singleton + mutex
G4Mutex OMSimSNAnalysis::m_mutex = G4Mutex();
OMSimSNAnalysis *OMSimSNAnalysis::m_instance = nullptr;

OMSimSNAnalysis &OMSimSNAnalysis::getInstance()
{
    if (!m_instance)
    {
        G4AutoLock lock(&m_mutex);
        if (!m_instance)
        {
            m_instance = new OMSimSNAnalysis();
        }
    }
    return *m_instance;
}

OMSimSNAnalysis::OMSimSNAnalysis()
 : m_outputSufix(OMSimCommandArgsTable::getInstance().get<std::string>("output_file")),
   m_shortInfo(OMSimCommandArgsTable::getInstance().get<bool>("ShortInfo"))
{
}

void OMSimSNAnalysis::initEventStat()
{
    m_eventStat = new SNEventStats();
}

void OMSimSNAnalysis::writeHeaders()
{
    log_trace("Writing headers");
    writeInfoFileHeader();
    writeDataFileHeader();
    m_headerWasWritten = true;
}

void OMSimSNAnalysis::writeInfoFileHeader()
{
    G4String fileName = m_outputSufix + "_" + Tools::getThreadIDStr() + "_info.dat";
    std::fstream dataFile(fileName, std::ios::out | std::ios::app);
    dataFile << "#" << G4endl;
    dataFile << "# Time of Flux [s] | Mean energy of nu/nubar | nu energy | "
             << "costheta of e-/e+ from z dir | e-/e+ energy | event weight" << G4endl;
    dataFile << "#" << G4endl;
}

void OMSimSNAnalysis::writeDataFileHeader()
{
    G4String fileName = m_outputSufix + "_" + Tools::getThreadIDStr() + "_data.dat";
    std::fstream dataFile(fileName, std::ios::out | std::ios::app);
    dataFile << "#" << G4endl;
    dataFile << "# Nr of modules | For each module, hit counts |";
    dataFile << " For each hit, starting with module 0: PMT nr \t hit time (ns) \t Detection probability ";
    dataFile << "#" << G4endl;
}

void OMSimSNAnalysis::processEvent(G4int eventID)
{
    log_trace("Processing SN event");
//    if (!m_headerWasWritten)
//    {
//        writeHeaders();
//    }

    writeInfoFile(eventID);
    writeDataFile(eventID);
}

void OMSimSNAnalysis::writeInfoFile(G4int eventID)
{

    OMSimHitManager &hitManager = OMSimHitManager::getInstance();

    bool anyHit = false;
    if (m_shortInfo)
    {
        for (int iModule = 0; iModule < hitManager.getNumberOfModules(); iModule++)
        {
            if (hitManager.areThereHitsInModuleSingleThread(iModule))
            {
                anyHit = true;
                break;
            }
        }

        // Skip writing if no hit and ShortInfo is enabled
        if (!anyHit)
            return;
    }
    log_trace("Writing SN event info file");
    G4String fileName = m_outputSufix + "_" + Tools::getThreadIDStr() + "_info.dat";

    std::fstream dataFile(fileName, std::ios::out | std::ios::app);
    dataFile << eventID << "\t";
    dataFile << m_eventStat->neutrinoTime / s << "\t";
    dataFile << m_eventStat->meanEnergy / MeV << "\t";
    dataFile << m_eventStat->neutrinoEnergy / MeV << "\t";
    dataFile << m_eventStat->cosTheta << "\t";
    dataFile << m_eventStat->primaryEnergy / MeV << "\t";
    dataFile << m_eventStat->weight << "\t";
    dataFile << m_eventStat->vertexDistance / m << "\n";
}

void OMSimSNAnalysis::writeDataFile(G4int eventID)
{
    OMSimHitManager &hitManager = OMSimHitManager::getInstance();

    bool anyHit = false;
    if (m_shortInfo)
    {
        for (int iModule = 0; iModule < hitManager.getNumberOfModules(); iModule++)
        {
            if (hitManager.areThereHitsInModuleSingleThread(iModule))
            {
                anyHit = true;
                break;
            }
        }

        // Skip writing if no hit and ShortInfo is enabled
        if (!anyHit)
            return;
    }
    
    log_trace("Writing SN event hit data file");

    G4String fileName = m_outputSufix + "_" + Tools::getThreadIDStr() + "_data.dat";
    std::fstream dataFile(fileName, std::ios::out | std::ios::app);
    
    dataFile << eventID << "\t";
    dataFile << hitManager.getNumberOfModules() << "\t";

    // write hit count for each module
    for (int iModule = 0; iModule < hitManager.getNumberOfModules(); iModule++)
    {
        G4double numberHits = 0;
        if (hitManager.areThereHitsInModuleSingleThread(iModule))
        {
            HitStats hits = hitManager.getSingleThreadHitsOfModule(iModule);
            numberHits = hits.eventId.size();
        }
        dataFile << numberHits << "\t";
    }

    // write hit information
    for (int iModule = 0; iModule < hitManager.getNumberOfModules(); iModule++)
    {
        if (hitManager.areThereHitsInModuleSingleThread(iModule))
        {
            HitStats hits = hitManager.getSingleThreadHitsOfModule(iModule);
            hitManager.sortHitStatsByTime(hits);
            for (int i = 0; i < (int)hits.eventId.size(); i++)
            {
                dataFile << hits.PMTnr.at(i) << "\t";   // FIX: no division by ns here
                dataFile << hits.hitTime.at(i) / ns << "\t";
                // dataFile << hits.generationDetectionDistance.at(i) / m << "\t";
                dataFile << hits.PMTresponse.at(i).detectionProbability << "\t";
            }
        }
    }
    dataFile << "\n";
}

//
// --- NEW: merging functions (modeled after OMSimDecaysAnalysis) ---
//
void OMSimSNAnalysis::mergeThreadFiles(G4String p_fileEnd)
{
    G4String mergedFileName = m_outputSufix + p_fileEnd;

    std::ofstream mergedFile(mergedFileName.c_str(), std::ios::out | std::ios::app);

    if (!mergedFile.is_open())
    {
        log_error("Failed to open merged SN file: {}", mergedFileName);
        return;
    }

    if (p_fileEnd == "_info.dat")
    {
        mergedFile << "#" << G4endl;
        mergedFile << "# EventID | Time of Flux [s] | Mean energy of nu/nubar | nu energy | "
                   << "costheta of e-/e+ from z dir | e-/e+ energy | event weight | Abs distance (m)" << G4endl;
        mergedFile << "#" << G4endl;
    }
    else if (p_fileEnd == "_data.dat")
    {
        mergedFile << "#" << G4endl;
        mergedFile << "#EventID | Nr of modules | For each module, hit counts |";
        mergedFile << " For each hit, starting with module 0: PMT nr \t hit time (ns) \t Detection probability ";
        mergedFile << "#" << G4endl;
    }

    int numThreads = OMSimCommandArgsTable::getInstance().get<int>("threads");

    for (int threadID = 0; threadID < numThreads; ++threadID)
    {
        G4String threadFileName = m_outputSufix + "_" + std::to_string(threadID) + p_fileEnd;

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

void OMSimSNAnalysis::mergeFiles()
{
    mergeThreadFiles(G4String("_info.dat"));
    mergeThreadFiles(G4String("_data.dat"));
}
