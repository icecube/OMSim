/** @file OMSimSNAnalysis.cc
 *  @brief Analyze data and write output files
 */

#include "OMSimSNAnalysis.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include <OMSimTools.hh>

G4ThreadLocal SNEventStats *OMSimSNAnalysis::m_eventStat = nullptr;
G4ThreadLocal bool OMSimSNAnalysis::m_headerWasWritten = false;

OMSimSNAnalysis::OMSimSNAnalysis() : m_outputSufix(OMSimCommandArgsTable::getInstance().get<std::string>("output_file"))
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
    std::fstream dataFile;
    dataFile.open(fileName.c_str(), std::ios::out | std::ios::app);
    dataFile << "#" << G4endl;
    dataFile << "# Time of Flux [s] | Mean energy of nu/nubar | nu energy | costheta of e-/e+ from z dir | e-/e+ energy | event weight" << G4endl;
    dataFile << "#" << G4endl;
    dataFile.close();
}

void OMSimSNAnalysis::writeDataFileHeader()
{
    G4String fileName = m_outputSufix + "_" + Tools::getThreadIDStr() + "_data.dat";
    std::fstream dataFile;
    dataFile.open(fileName.c_str(), std::ios::out | std::ios::app);
    dataFile << "#" << G4endl;
    dataFile << "# Nr of modules | For each module, hit counts |";
    dataFile << " For each hit, starting with module 0: PMT nr \t hit time (ns) \t Detection probability ";
    dataFile << "#" << G4endl;
    dataFile.close();
}

void OMSimSNAnalysis::processEvent()
{
    log_trace("Processing SN event");
    if (!m_headerWasWritten)
    {
        writeHeaders();
    }

    writeInfoFile();
    writeDataFile();
}

void OMSimSNAnalysis::writeInfoFile()
{
    log_trace("Writing SN event info file");
    G4String fileName = m_outputSufix + "_" + Tools::getThreadIDStr() + "_info.dat";

    std::fstream dataFile;
    dataFile.open(fileName.c_str(), std::ios::out | std::ios::app);
    dataFile << m_eventStat->neutrinoTime / s << "\t";
    dataFile << m_eventStat->meanEnergy / MeV << "\t";
    dataFile << m_eventStat->neutrinoEnergy / MeV << "\t";
    dataFile << m_eventStat->cosTheta << "\t";
    dataFile << m_eventStat->primaryEnergy / MeV << "\t";
    dataFile << m_eventStat->weight << "\n";
    dataFile.close();
}

void OMSimSNAnalysis::writeDataFile()
{
    log_trace("Writing SN event hit data file");
    OMSimHitManager &hitManager = OMSimHitManager::getInstance();

    G4String fileName = m_outputSufix + "_" + Tools::getThreadIDStr() + "_data.dat";
    std::fstream dataFile;
    dataFile.open(fileName.c_str(), std::ios::out | std::ios::app);
    
    dataFile << hitManager.getNumberOfModules() << "\t";

    //write hit count for each module
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

    //write hit information
    for (int iModule = 0; iModule < hitManager.getNumberOfModules(); iModule++)
    {
        if (hitManager.areThereHitsInModuleSingleThread(iModule))
        {
            HitStats hits = hitManager.getSingleThreadHitsOfModule(iModule);
            hitManager.sortHitStatsByTime(hits);
            for (int i = 0; i < (int)hits.eventId.size(); i++){
                dataFile << hits.PMTnr.at(i) / ns << "\t";
                dataFile << hits.hitTime.at(i) / ns << "\t";
                dataFile << hits.PMTresponse.at(i).detectionProbability << "\t";
            }
        }
    }
    dataFile << "\n";
    dataFile.close();
}
