/** @file OMSimSNAnalysis.cc
 *  @brief Analyze data and write output files
 * 
 *  @author Lew Classen (lclassen@wwu.de), Cristian Jesus Lozano Mariscal (c.lozano@wwu.de)
 * 
 *  @version Geant4 10.7
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
    std::vector<double> lHits = OMSimHitManager::getInstance().countHits();
    //G4cout << lHits << G4endl;
    if ((G4int)hitStats.size() > 0) {
        Writer_InfoFile();
        Helper_AnalyzeEvent(evtStat0);
        Writer_data(datafile,evtStat0);
    }
}


void OMSimSNAnalysis::Helper_AnalyzeEvent(EvtStat& this_evtStat)
{
    int gn_mDOMs = 1; //TODO make this general! 
    std::vector<G4int> modulescounter;
    modulescounter.resize(gn_mDOMs);
    for (int k=0; k<gn_mDOMs; k++) {
        for (int i=0; i<24; i++) {
            std::tuple<G4int,G4int,G4int> hitsPMT;
            std::get<0>(hitsPMT) = k;   
            std::get<1>(hitsPMT) = i;   
            std::get<2>(hitsPMT) = 0; 
            G4bool hit = false;
            for ( int j=0; j<(G4int)hitStats.size(); j++ ) {
                if ( (hitStats[j].moduleNr == k) && (hitStats[j].pmtNr == i) ) {
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
        if (std::get<2>(hitsPMT) != 0 ) this_evtStat.hitsPMTs.push_back(hitsPMT);
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

void OMSimSNAnalysis::Writer_data(std::fstream& thisfile, EvtStat& this_evtStat)
{
    if (OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy") == false) {
        thisfile << this_evtStat.nrHitTot << "\t";
        thisfile << this_evtStat.nrHitMod << "\t";
        thisfile << this_evtStat.nrHitPMTs << "\t";
        thisfile << "\t";
        for ( int j=0; j<(G4int)this_evtStat.hitsPMTs.size(); j++ ) {
            thisfile << std::get<0>(this_evtStat.hitsPMTs[j]) << "\t";
            thisfile << std::get<1>(this_evtStat.hitsPMTs[j]) << "\t";
            thisfile << std::get<2>(this_evtStat.hitsPMTs[j]) << "\t";
            G4double PMTprob= 0;
            for (int i=0; i<(G4int)hitStats.size(); i++) {
                if ( (std::get<0>(this_evtStat.hitsPMTs[j]) == hitStats[i].moduleNr) && (std::get<1>(this_evtStat.hitsPMTs[j]) == hitStats[i].pmtNr) ) {
                    thisfile << hitStats[i].hit_time/ns << "\t";
                }
            }
        }
        thisfile << G4endl;
    } else {
        thisfile << this_evtStat.nrHitPMTs << "\t";
        thisfile << weigh << "\t";
        thisfile << nuEnergy/MeV<< "\t";
        thisfile << G4endl;
    }
}
