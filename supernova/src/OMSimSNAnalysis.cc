/** @file OMSimSNAnalysis.cc
 *  @brief Analyze data and write output files
 */

#include "OMSimSNAnalysis.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"

G4ThreadLocal SNEventStats *OMSimSNAnalysis::mEventStat = nullptr;
G4ThreadLocal bool OMSimSNAnalysis::mHeaderWasWritten = false;

OMSimSNAnalysis::OMSimSNAnalysis() : mOutputSuffix(OMSimCommandArgsTable::getInstance().get<std::string>("output_file"))
{
}

void OMSimSNAnalysis::initEventStat()
{
    mEventStat = new SNEventStats();

}

void OMSimSNAnalysis::writeHeaders()
{
    log_trace("Writing headers");
    writeInfoFileHeader();
    writeDataFileHeader();
    mHeaderWasWritten = true;
}

void OMSimSNAnalysis::writeInfoFileHeader()
{
    G4String lFileName = mOutputSuffix + "_" + OMSimHitManager::getInstance().getThreadIDStr() + "_info.dat";
    std::fstream lDatafile;
    lDatafile.open(lFileName.c_str(), std::ios::out | std::ios::app);
    lDatafile << "#" << G4endl;
    lDatafile << "# Time of Flux [s] | Mean energy of nu/nubar | nu energy | costheta of e-/e+ from z dir | e-/e+ energy | event weight" << G4endl;
    lDatafile << "#" << G4endl;
    lDatafile.close();
}

void OMSimSNAnalysis::writeDataFileHeader()
{
    G4String lFileName = mOutputSuffix + "_" + OMSimHitManager::getInstance().getThreadIDStr() + "_data.dat";
    std::fstream lDatafile;
    lDatafile.open(lFileName.c_str(), std::ios::out | std::ios::app);
    lDatafile << "#" << G4endl;
    lDatafile << "# Nr of modules | For each module, hit counts |";
    lDatafile << " For each hit, starting with module 0: PMT nr \t hit time (ns) \t Detection probability ";
    lDatafile << "#" << G4endl;
    lDatafile.close();
}

void OMSimSNAnalysis::processEvent()
{
    log_trace("Processing SN event");
    if (!mHeaderWasWritten)
    {
        writeHeaders();
    }

    writeInfoFile();
    writeDataFile();
}

void OMSimSNAnalysis::writeInfoFile()
{
    log_trace("Writing SN event info file");
    G4String lFileName = mOutputSuffix + "_" + OMSimHitManager::getInstance().getThreadIDStr() + "_info.dat";

    std::fstream lDatafile;
    lDatafile.open(lFileName.c_str(), std::ios::out | std::ios::app);
    lDatafile << mEventStat->neutrino_time / s << "\t";
    lDatafile << mEventStat->mean_energy / MeV << "\t";
    lDatafile << mEventStat->neutrino_energy / MeV << "\t";
    lDatafile << mEventStat->cos_theta << "\t";
    lDatafile << mEventStat->primary_energy / MeV << "\t";
    lDatafile << mEventStat->weight << "\n";
    lDatafile.close();
}

void OMSimSNAnalysis::writeDataFile()
{
    log_trace("Writing SN event hit data file");
    OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

    G4String lFileName = mOutputSuffix + "_" + lHitManager.getThreadIDStr() + "_data.dat";
    std::fstream lDatafile;
    lDatafile.open(lFileName.c_str(), std::ios::out | std::ios::app);
    
    lDatafile << lHitManager.getNumberOfModules() << "\t";

    for (int iModule = 0; iModule < lHitManager.getNumberOfModules(); iModule++)
    {
        G4double lNrHits = 0;
        if (lHitManager.areThereHitsInModuleSingleThread(iModule))
        {
            HitStats lHits = lHitManager.getSingleThreadHitsOfModule(iModule);
            lNrHits = lHits.eventId.size();
        }

        lDatafile << lNrHits << "\t";
    }

    for (int iModule = 0; iModule < lHitManager.getNumberOfModules(); iModule++)
    {
        if (lHitManager.areThereHitsInModuleSingleThread(iModule))
        {
            HitStats lHits = lHitManager.getSingleThreadHitsOfModule(iModule);
            lHitManager.sortHitStatsByTime(lHits);
            for (int i = 0; i < (int)lHits.eventId.size(); i++){
                lDatafile << lHits.PMTnr.at(i) / ns << "\t";
                lDatafile << lHits.hitTime.at(i) / ns << "\t";
                lDatafile << lHits.PMTresponse.at(i).detectionProbability << "\t";
            }
        }
    }
    lDatafile << "\n";
    lDatafile.close();
}
