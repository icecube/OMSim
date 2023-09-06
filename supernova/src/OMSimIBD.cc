#include "OMSimIBD.hh"
#include "OMSimSNAnalysis.hh"
#include "OMSimCommandArgsTable.hh"

#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleTypes.hh"

#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "OMSimSNTools.hh"

extern double gworldsize;
extern G4String	gnubarfluxname;
extern std::vector<double> readColumnDouble (G4String fn, int col);
//extern MdomAnalysisManager gAnalysisManager;

extern G4double	gSNmeanEnergy;
extern G4double 	gfixalpha;

OMSimIBD::OMSimIBD(G4ParticleGun* gun)
: ParticleGun(gun)
{        
  // building energy distribution of electronic antineutrinos...
  //
  if ((OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy")) == false) {
    nubar_time = readColumnDouble(gnubarfluxname, 1);
    nubar_luminosity = readColumnDouble(gnubarfluxname, 2);
    nubar_meanenergy = readColumnDouble(gnubarfluxname, 3);
    nubar_meanenergysquare = readColumnDouble(gnubarfluxname, 4);

	for (unsigned int u = 0; u <nubar_time.size(); u++) {
		nubar_time[u] = nubar_time.at(u)*s;
		nubar_meanenergy[u] = nubar_meanenergy.at(u)*MeV;
		nubar_meanenergysquare[u] = nubar_meanenergysquare.at(u)*MeV*MeV;
		}
	// Since the luminosity spectrum is not gonna change, it is worthy to compute already the slopes and store them
	nPoints_lum =  nubar_time.size();
	GetSlopes(nubar_time,  nubar_luminosity, nPoints_lum, x_lum, f_lum, a_lum, Fc_lum);

  } else {
	mFixedenergy = OMSimCommandArgsTable::getInstance().get<G4double>("SNmeanE") * MeV;
	mAlpha = OMSimCommandArgsTable::getInstance().get<G4double>("SNalpha"); 
	mFixedenergy2 = mFixedenergy*mFixedenergy*(2+mAlpha)/(1+mAlpha); //Only for crosscheck
	std::vector<G4double> x1;
	std::vector<G4double> f1;	
	fixE_nPoints = 500;
	MakeEnergyDistribution(mFixedenergy, mAlpha, fixE_nPoints, x1, f1);
	GetSlopes(x1, f1, fixE_nPoints, fixFe_X, fixFe_Y, fixFe_a, fixFe_Fc);
  }
      
  Gf = 1.166e-5*1e-6/(MeV*MeV);
  me = electron_mass_c2;
  mp = proton_mass_c2;
  mn = neutron_mass_c2;
  consg = 1.26;

  NTargets = NumberOfTargets(2); //2 protons (hydrogen) per molecule

}



OMSimIBD::~OMSimIBD()
{ }



void OMSimIBD::GeneratePrimaries(G4Event* anEvent)
{

    // Particle and position
  G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("e+");
  ParticleGun->SetParticleDefinition(particle);
  
  OMSimSNTools SNToolBox;

  G4ThreeVector Position = SNToolBox.RandomPosition();
  ParticleGun->SetParticlePosition(Position);
  //G4cout << Position << G4endl;
  beggining:


  //set energy from a tabulated distribution
  //
  G4double timeofspectrum;
  G4double Emean;
  G4double Emean2;
  G4double alpha;
  G4double nubar_energy;
  if ((OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy")) == false) {
    timeofspectrum = SNToolBox.InverseCumulAlgorithm(x_lum, f_lum, a_lum, Fc_lum,nPoints_lum);
    
    G4int timepos = SNToolBox.findtime(timeofspectrum, nubar_time);
    Emean = SNToolBox.linealinterpolation(timeofspectrum,nubar_time.at(timepos-1), nubar_time.at(timepos), nubar_meanenergy.at(timepos-1),nubar_meanenergy.at(timepos));
    Emean2 = SNToolBox.linealinterpolation(timeofspectrum,nubar_time.at(timepos-1), nubar_time.at(timepos), nubar_meanenergysquare.at(timepos-1),nubar_meanenergysquare.at(timepos));

    ThresholdEnergy = neutron_mass_c2 + electron_mass_c2 - proton_mass_c2+0.1*MeV; //+0.1MeV because angularcrosssection fails if the energy is too close to the threshold.
    nubar_energy = 0;

    G4int count = 0;
    while (nubar_energy <= ThresholdEnergy) {
      nubar_energy = SNToolBox.EnergyDistribution(Emean, Emean2, alpha);
      count += 1;
      if (count > 10) {
        goto beggining;
        // To avoid entering in an almost infinity bucle if energy of the chosen time is very unlikely to be higher than 
        // the threshold, go to the beggining and choose a different time of the burst
      }
    }
  } else {
    timeofspectrum = 0.0;
    Emean = mFixedenergy;
    Emean2 = mFixedenergy2;
    nubar_energy =  SNToolBox.InverseCumulAlgorithm(fixFe_X, fixFe_Y, fixFe_a, fixFe_Fc, fixE_nPoints);
          while (nubar_energy <= ThresholdEnergy) {
              //notice that, if fixE is very low, we might get stuck here a while for each particle
              nubar_energy =  SNToolBox.InverseCumulAlgorithm(fixFe_X, fixFe_Y, fixFe_a, fixFe_Fc, fixE_nPoints);
          }
  }

  // angle distribution. We suppose the incident antineutrino would come with momentum direction (0,0,-1)
  DistFunction(nubar_energy);
  
  G4double costheta = SNToolBox.InverseCumul(angdist_x, angdist_y, angdist_nPoints);
  G4double sintheta = std::sqrt(1. - costheta*costheta);
  G4double phi = twopi*G4UniformRand();
    
  G4double zdir = -costheta; 
  G4double xdir = -sintheta*std::cos(phi);
  G4double ydir = -sintheta*std::sin(phi);
  
  // from nu_energy and costheta, we get e- energy
  G4double p_energy = PositronEnergy(nubar_energy, costheta);
  
  ParticleGun->SetParticleEnergy(p_energy); 
  ParticleGun->SetParticleMomentumDirection(G4ThreeVector(xdir,ydir,zdir));
  
  G4double sigma = TotalCrossSection(nubar_energy);
  G4double Weigh = SNToolBox.WeighMe(sigma, NTargets);
  
  //sending stuff to analysismanager
  OMSimSNAnalysis &lAnalysisManager = OMSimSNAnalysis::getInstance();
  lAnalysisManager.nuTime = timeofspectrum;
  lAnalysisManager.nuMeanEnergy = Emean;
  lAnalysisManager.nuEnergy = nubar_energy;
  lAnalysisManager.cosTheta = costheta;
  lAnalysisManager.primaryEnergy = p_energy; 
  lAnalysisManager.weigh = Weigh;

  
  // G4cout << timeofspectrum<< "      " << nubar_energy/MeV << "        " << p_energy/MeV << "        " << costheta << G4endl;
  //create vertex
  //   
  ParticleGun->GeneratePrimaryVertex(anEvent);
}

void OMSimIBD::DistFunction(G4double Enubar)
{
  // Angular distribution
  //
  // P. Vogel, J. F. Beacom. (1999). The angular distribution of the reaction nubar_e + p -> e+ + n. Phys.Rev., D60, 053003 
  // Eq. 14
  //
  angdist_nPoints = 300;
 
  G4double costhetac = 0.974;
  G4double consf = 1.;
  G4double consf2 = 3.706;
  Delta = mn-mp;
  y2 = (pow(Delta,2)-pow(me,2))/2.;
  G4double M = mp;
  G4double sigma0 = pow(Gf,2)*pow(costhetac,2)/pi*1.024*pow(197.326e-15,2);

  angdist_x.resize(angdist_nPoints); 
  angdist_y.resize(angdist_nPoints);
  
  G4double min = -1.; G4double max = 1.; 
  G4double delta = (max-min)/G4double(angdist_nPoints-1);
  for (G4int i=0; i<angdist_nPoints; i++) {
    angdist_x[i] = min + i*delta; //costheta
  }
  
    G4double Ee0 = Enubar - Delta;
    G4double pe0 = pow((pow(Ee0,2)-pow(me,2)),1./2.);
    G4double ve0 = pe0/Ee0;
  
  for (G4int j=0; j<angdist_nPoints; j++) {
    G4double Ee1 = Ee0*(1.-Enubar/M*(1.-ve0*angdist_x[j]))-y2/M;
    G4double pe1 = pow((pow(Ee1,2)-pow(me,2)),1./2.);
    G4double ve1 = pe1/Ee1;
    G4double part1 = 2.*(consf+consf2)*consg*((2.*Ee0+Delta)*(1.-ve0*angdist_x[j])-pow(me,2)/Ee0);
    G4double part2 = (pow(consf,2)+pow(consg,2))*(Delta*(1.+ve0*angdist_x[j])+pow(me,2)/Ee0);
    G4double part3 = (pow(consf,2)+3.*pow(consg,2))*((Ee0+Delta)*(1.- angdist_x[j]/ve0)-Delta);
    G4double part4 = (pow(consf,2)+pow(consg,2))*((Ee0+Delta)*(1.-angdist_x[j]/ve0)-Delta)*ve0*angdist_x[j];
    G4double Tau = part1 + part2 + part3 + part4;

    angdist_y[j] = sigma0/2.*((pow(consf,2)+3.*pow(consg,2))+(pow(consf,2)-pow(consg,2))*ve1*angdist_x[j])*Ee1*pe1-sigma0/2.*(Tau/M)*Ee0*pe0;// dsigma/dcos(theta)
  }
}


G4double OMSimIBD::PositronEnergy(G4double Enubar, G4double costheta)
{
  // Get positron energy from inverse beta decay as a function of incident antineutrino energy and scatter angle
  //
  // P. Vogel, J. F. Beacom. (1999). The angular distribution of the reaction nu_e + p -> e+ + n. Phys.Rev., D60, 053003 
  // Eq. 13
  //
  G4double Ee0 = Enubar - Delta;
  G4double pe0 = pow((pow(Ee0,2)-pow(electron_mass_c2,2)),1./2.);
  G4double ve0 = pe0/Ee0;
  G4double Energy = Ee0*(1.-Enubar/proton_mass_c2*(1.-ve0*costheta))-y2/proton_mass_c2;
  return Energy;
}



G4double OMSimIBD::TotalCrossSection(G4double energy) {
  // Returns value of the TotalCrossSection for certain energy to use it in WeighMe
  //
  // T. Totani, K. Sato, H. E. Dalhed, J. R. Wilson,Future detection of supernova neutrino burst and explosion mechanism, Astrophys. J. 496 ,1998 216???225, Preprint astro-ph/9710203, 1998, Equation 9
  G4double hbar = 6.58211899e-16*1e-6*MeV*s;
  G4double c = 3e8*m/s;
  G4double sigma0 = pow(2.*Gf*me*hbar*c,2)/pi;
  G4double constante = -0.00325/(MeV*MeV);
  G4double deltaWM = constante*(energy-(mn-mp)/2.);
  G4double Ee = energy+mp-mn;
  G4double pec = pow((pow(Ee,2)-pow(me,2)),1./2.);
  G4double sigma = 1./4.*sigma0*(1.+3.*pow(consg,2))*(1.+deltaWM)*Ee*pec/pow(me,2);
  return sigma;
}


