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



void OMSimSNAnalysis::AnalyzeEvent() {
    //G4cout << (G4int)hitStats.size() << G4endl;
    /*
    G4int counter = 0;
    for (int i=0; i<(G4int)AllFamilyTracks.size(); i++ ) {
        counter = counter + 1;
        FamilyTrack thisfamilytrack = AllFamilyTracks.at(i);
        G4cout << "** Family track number "<< counter << G4endl;
        G4cout << "Mother Particle -> " << thisfamilytrack.motherparticle << " with size " << thisfamilytrack.tracks.size() <<G4endl;
        /*
        for (int j=0; j<(G4int)thisfamilytrack.tracks.size(); j++ ) {
            G4cout << "track check -- " <<thisfamilytrack.tracks.at(j) << G4endl; 
        } 
    } */
    
    if ((G4int)hitStats.size() > 0) {
        Writer_InfoFile();
        Helper_AnalyzeEvent(evtStat0);
        Writer_data(datafile,evtStat0);
    }
}


void OMSimSNAnalysis::Helper_AnalyzeEvent(EvtStat& this_evtStat)
{
    int gn_mDOMs = 1; //TODO makes this general! 
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

void OMSimSNAnalysis::WriteHeader()
{
    maininfofile << "test info" << G4endl;
    datafile << "test data" << G4endl;
}

void OMSimSNAnalysis::HelpTheHeader(std::fstream& thisfile)
{
    thisfile << "# Total hits | Modules hit | PMTs hit |";
    if (gQEweigh) {
        thisfile << " Total QE prob |";
    }
    thisfile <<"...for PMT hit...| Module number | PMT number | Hits in that PMT |";
    thisfile << "...for Hit...";
    if (gQEweigh) {
        thisfile << " QE prob |";
    }
    thisfile << " hit time |";
    if (gQEweigh) {
        thisfile << "...end of hit loop...| Total QE prob in PMT |" << G4endl;
    }
    thisfile << "#"<< G4endl;
}


void OMSimSNAnalysis::Writer_InfoFile() {
      if (OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy") == false) {
        maininfofile << nuTime/s << "\t";
        maininfofile << nuMeanEnergy/MeV<< "\t";
        maininfofile << nuEnergy/MeV<< "\t";
        maininfofile << cosTheta<< "\t";
        maininfofile << primaryEnergy/MeV << "\t";
        maininfofile << weigh << "\t\t";
      }
}

void OMSimSNAnalysis::Writer_data(std::fstream& thisfile, EvtStat& this_evtStat)
{
      if (OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy") == false) {

        thisfile << this_evtStat.nrHitTot << "\t";
        thisfile << this_evtStat.nrHitMod << "\t";
        thisfile << this_evtStat.nrHitPMTs << "\t";
        if (gQEweigh) {
            G4double sum = 0;
            for (unsigned int i = 0 ; i<hitStats.size(); i++) {
                sum = sum + hitStats[i].QEprob;
            }
            thisfile << sum << "\t";
            
        }
        thisfile << "\t";
        for ( int j=0; j<(G4int)this_evtStat.hitsPMTs.size(); j++ ) {
            thisfile << std::get<0>(this_evtStat.hitsPMTs[j]) << "\t";
            thisfile << std::get<1>(this_evtStat.hitsPMTs[j]) << "\t";
            thisfile << std::get<2>(this_evtStat.hitsPMTs[j]) << "\t";
            G4double PMTprob= 0;
            for (int i=0; i<(G4int)hitStats.size(); i++) {
                if ( (std::get<0>(this_evtStat.hitsPMTs[j]) == hitStats[i].moduleNr) && (std::get<1>(this_evtStat.hitsPMTs[j]) == hitStats[i].pmtNr) ) {
                    //thisfile << hitStats[i].wavelen/nm << "\t";
                    if (gQEweigh) {
                        PMTprob = PMTprob + hitStats[i].QEprob;
                        thisfile << hitStats[i].QEprob << "\t";
                        }
                    thisfile << hitStats[i].hit_time/ns << "\t";
                    }
            if (gQEweigh) {
                thisfile << PMTprob << "\t\t";
                }
            }
        }
        thisfile << G4endl;
      } else {
        thisfile << this_evtStat.nrHitPMTs << "\t";
        thisfile  << weigh << "\t";
        thisfile << nuEnergy/MeV<< "\t";
        thisfile << G4endl;
    }
}
