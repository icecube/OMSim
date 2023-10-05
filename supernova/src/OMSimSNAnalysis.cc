/** @file OMSimSNAnalysis.cc
 *  @brief Analyze data and write output files
 * 
 *  @author Cristian Jesus Lozano Mariscal
 */

#include "OMSimSNAnalysis.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"


OMSimSNAnalysis::OMSimSNAnalysis(){
  }

OMSimSNAnalysis::~OMSimSNAnalysis(){
}

void OMSimSNAnalysis::ResetEvent()
{
	foundPhoton = false;
	photonTheta = -99.;
	photonPhi = -99.;
	photonR = -99.;
    hitStats.clear();
    Helper_ResetEvent(evtStat0);
    OMSimHitManager::getInstance().reset();
}

void OMSimSNAnalysis::Helper_ResetEvent(EvtStat& this_evtStat)
{
    this_evtStat.nrHitPMTs = 0;
    this_evtStat.nrHitTot = 0;
    this_evtStat.nrHitMod = 0;
    this_evtStat.hitsPMTs.clear();
}

void OMSimSNAnalysis::WriteHeaders() {
    InfoHeader();
    DataHeader();
}

void OMSimSNAnalysis::InfoHeader()
{
    if (OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy") == false) {
        maininfofile << "#"<< G4endl;
        maininfofile << "# Time of Flux [s] | Mean energy of nu/nubar | nu energy | costheta of e-/e+ from z dir | e-/e+ energy | event weight" << G4endl;
        /*test
        maininfofile << "| Vertex Position (X, Y, Z) [m] | Primary direction (Px,Py,Pz)" << G4endl;
        */
        maininfofile << "#" << G4endl;
    }
}

void OMSimSNAnalysis::DataHeader()
{
    if (OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy") == false) {
        datafile << "# Total hits | Modules hit | PMTs hit |";
        datafile <<"...for PMT hit...| Module number | PMT number | Hits in that PMT |";
        datafile << "...for Hit...";
        datafile << " hit time |";
        datafile << "#"<< G4endl;
    }
}

void OMSimSNAnalysis::AnalyzeEvent() {
    std::map<G4int, HitStats> lModuleHits = OMSimHitManager::getInstance().mModuleHits;
    G4int lNumberOfModulesHit = lModuleHits.size();
    bool hit = false;
    if (lNumberOfModulesHit > 0) {
        Writer_InfoFile();
        Helper_AnalyzeEvent(evtStat0, lModuleHits);
        Writer_data(evtStat0, lModuleHits);
    }
}

void OMSimSNAnalysis::Helper_AnalyzeEvent(EvtStat& this_evtStat, std::map<G4int, HitStats> pModuleHits)
{
    G4int lNumberOfModulesHit = pModuleHits.size();
    std::vector<G4int> modulescounter;
    modulescounter.resize(lNumberOfModulesHit);

    G4int maxOmNumber = (--pModuleHits.end())->first;
    for (int k=0; k<maxOmNumber+1; k++) {
        for (int i=0; i<24; i++) {
            std::tuple<G4int,G4int,G4int> hitsPMT;
            std::get<0>(hitsPMT) = k;   //module number
            std::get<1>(hitsPMT) = i;   //PMT number  
            std::get<2>(hitsPMT) = 0;   // # of hits in this PMT
            G4bool hit = false;
            for(const auto& pair : pModuleHits) {
                G4int lOmNumber = pair.first;
                const HitStats& lHitInfo = pair.second;
                if (lOmNumber == k) {
                    for (size_t hitindex = 0; hitindex < lHitInfo.hit_time.size(); ++hitindex) {
                        if (lHitInfo.PMT_hit[hitindex] == i) {
                        this_evtStat.nrHitTot += 1;
                        std::get<2>(hitsPMT) += 1; 
                        if ( ! hit ) {
                            this_evtStat.nrHitPMTs += 1;
                            if (modulescounter.at(k) == 0) {
                                modulescounter.at(k) += 1;
                                this_evtStat.nrHitMod += 1;
                                }
                            hit = true;
                            }
                        }
                    }
                }
            }
        if (std::get<2>(hitsPMT) != 0 ) {
            this_evtStat.hitsPMTs.push_back(hitsPMT);
            }
        }
    }
}


void OMSimSNAnalysis::Writer_InfoFile() {
    if (OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy") == false) {
        maininfofile << nuTime/s << "\t";
        maininfofile << nuMeanEnergy/MeV<< "\t";
        maininfofile << nuEnergy/MeV<< "\t";
        maininfofile << cosTheta<< "\t";
        maininfofile << primaryEnergy/MeV << "\t";
        maininfofile << weigh << "\n";
        /* //Tests
        maininfofile << primaryX/m << "\t";
        maininfofile << primaryY/m << "\t";
        maininfofile << primaryZ/m << "\t";
        maininfofile << primaryDirX << "\t";
        maininfofile << primaryDirY << "\t";
        maininfofile << primaryDirZ << "\t";
        */
    }
}

void OMSimSNAnalysis::Writer_data(EvtStat& this_evtStat, std::map<G4int, HitStats> pModuleHits)
{
    if (OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy") == false) {
        datafile << this_evtStat.nrHitTot << "\t";
        datafile << this_evtStat.nrHitMod << "\t";
        datafile << this_evtStat.nrHitPMTs << "\t";
        datafile << "\t";
        for ( int j=0; j<(G4int)this_evtStat.hitsPMTs.size(); j++ ) {
            datafile << std::get<0>(this_evtStat.hitsPMTs[j]) << "\t";
            datafile << std::get<1>(this_evtStat.hitsPMTs[j]) << "\t";
            datafile << std::get<2>(this_evtStat.hitsPMTs[j]) << "\t";

            for(const auto& pair : pModuleHits) {
                G4int lOmNumber = pair.first;
                const HitStats& lHitInfo = pair.second;
                if (lOmNumber == std::get<0>(this_evtStat.hitsPMTs[j])) {
                    for (size_t hitindex = 0; hitindex < lHitInfo.hit_time.size(); ++hitindex) {
                        if (lHitInfo.PMT_hit[hitindex] == std::get<1>(this_evtStat.hitsPMTs[j])) {
                            datafile << lHitInfo.hit_time[hitindex]/ns << "\t";
                        }
                    }   
                }
            }
        }
        datafile << G4endl;
    }
}
