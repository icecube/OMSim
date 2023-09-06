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

extern G4double gposX;
extern G4double gposY;
extern G4double gposZ;
extern G4double gtheta;
extern G4double gphi;
extern G4double 	gtMin, 	gfMin;
extern G4double gRadius;
extern G4double gHeight;
extern G4bool gQE;
extern G4bool gQEweigh;
extern G4int 	gSNGun;
extern G4double	gDistance;

extern G4int	gn_mDOMs;


extern G4bool		gfixmeanenergy;


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
    G4String part;
    G4String nu;
    //datafile << "#Generated in a cylinder of r = " << gRadius/m<< "m and total height of "<< 2*gHeight/m << " m"<< G4endl;
    if (gfixmeanenergy == false) {

    if (gSNGun == 1){
        maininfofile<< "# World with "<< gn_mDOMs << " mDOMs" << G4endl;
            maininfofile << "# Neutrino electron elastic scattering from SN at 10kpc" << G4endl;
            part = "e-";
            nu = "nu";
    } else if (gSNGun == 2){
        maininfofile<< "# World with "<< gn_mDOMs << " mDOMs" << G4endl;
            maininfofile << "# Inverse beta decay from SN at 10kpc" << G4endl;
            part = "e+";
            nu = "nubar";
    } else if (gSNGun == 3){
        maininfofile<< "# World with "<< gn_mDOMs << " mDOMs" << G4endl;
            maininfofile << "# Solar neutrinos! Weigh take into account the total cross section but not the flux!" << G4endl;
            part = "e-";
            nu = "nu";
    }
    }
    if (gSNGun == 0) {
        realdistance = gDistance*m;// - ((0.5 * 356.0)/1000.0)*m;
        datafile << "# World with "<< gn_mDOMs << " mDOMs" << G4endl;
        datafile << "# Gammas at "<< realdistance/m << " meters" <<G4endl;
        datafile << "# RealDistance [m] | Vertex Position (X, Y, Z) [m] | Primary direction (Px,Py,Pz)  |Total hits | ModulesHit | PMTsHit |";
        if (gQEweigh) {
            datafile << " Total QE prob |";
            }
            datafile <<"...for PMT hit...| Module number | PMT number | Hits in that PMT |";
            datafile << "...for Hit...";
            if (gQEweigh) {
                    datafile << " QE prob |";
            }
            datafile << " hit time |";
            if (gQEweigh) {
                datafile << "...end of hit loop...| Total QE prob in PMT |" << G4endl;
            }
            datafile << "#"<< G4endl;
            datafile << "#"<< G4endl;
        }
    else if (gSNGun == 1 || gSNGun == 2  || gSNGun == 3) {
          if (gfixmeanenergy == false) {

        maininfofile << "#"<< G4endl;
        maininfofile << "# Time of Flux [s] | Mean energy of "<<nu<<" | "<<nu<<" energy | costheta of "<<part<<" from z dir | "<<part<<" energy | event weigh | Vertex Position (X, Y, Z) [m] | Primary direction (Px,Py,Pz)" << G4endl;
        maininfofile << "#" << G4endl;
        
        HelpTheHeader(datafile);
          }
    } else {
        datafile << "# PMTs hit | weight | nu_energy";
    }
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
      if (gfixmeanenergy == false) {

      if (gSNGun == 1 || gSNGun == 2 || gSNGun == 3)  {
        maininfofile << nuTime/s << "\t";
        maininfofile << nuMeanEnergy/MeV<< "\t";
        maininfofile << nuEnergy/MeV<< "\t";
        maininfofile << cosTheta<< "\t";
        maininfofile << primaryEnergy/MeV << "\t";
        maininfofile << weigh << "\t\t";
      }
     if (gSNGun == 0)  {
        maininfofile << realdistance/m << "\t";
      }
        maininfofile << primaryX/m <<"\t";
        maininfofile << primaryY/m << "\t";
        maininfofile << primaryZ/m << "\t\t";
        maininfofile << primaryDirX <<"\t";
        maininfofile << primaryDirY << "\t";
        maininfofile << primaryDirZ << "\t\t";
    maininfofile << G4endl;
      }
}

void OMSimSNAnalysis::Writer_data(std::fstream& thisfile, EvtStat& this_evtStat)
{
      if (gfixmeanenergy == false) {

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
