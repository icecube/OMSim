#include "OMSimENES.hh"
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
#include "OMSimInputData.hh"

OMSimENES::OMSimENES(G4ParticleGun* gun)
: ParticleGun(gun)
{ 
  // building energy distribution of electronic neutrinos...
  //
  if ((OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy")) == false) {
    std::pair<std::string, std::string> lFluxNames = mSNToolBox.getFileNames(OMSimCommandArgsTable::getInstance().get<G4int>("SNtype"));
    std::string lnuFluxName = lFluxNames.first;
    //load model data
    std::vector<std::vector<double>> lData = InputDataManager::loadtxt(lnuFluxName, true, 0, '\t');
    //for luminosity, only the curve is needed and not the units.
    //the total flux must be accounted for by the user and the events properly weigh
    mNu_luminosity = lData[1]; //second column is luminosity
    //for the rest, we need to specify the units. Make sure if you use a new models that the units are the same
    mNu_time.resize(lData[0].size());
    mNu_luminosity.resize(lData[1].size());
    mNu_meanenergy.resize(lData[2].size());
    mNu_meanenergysquare.resize(lData[3].size());

    for (unsigned int u = 0; u <lData[0].size(); u++) {
      mNu_time[u] = lData[0].at(u)*s; //first column corresponds to time
      mNu_meanenergy[u] = lData[2].at(u)*MeV; //3rd column corresponds to meanE
      mNu_meanenergysquare[u] = lData[3].at(u)*MeV*MeV; //4th column corresponds to squaredmeanE
    }

    // Since the luminosity spectrum is not gonna change, it is worthy to compute already the slopes and store them
    nPoints_lum =  mNu_time.size();
    GetSlopes(mNu_time,  mNu_luminosity, nPoints_lum, x_lum, f_lum, a_lum, Fc_lum);

  } else { //save all slopes and stuff since it always gonna be the same
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
  
  NTargets = NumberOfTargets(10); //10 electrons per molecule
}



OMSimENES::~OMSimENES()
{ }



void OMSimENES::GeneratePrimaries(G4Event* anEvent)
{
  // Particle and position
  G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle("e-");
  ParticleGun->SetParticleDefinition(particle);
  G4ThreeVector Position = mSNToolBox.RandomPosition();
  ParticleGun->SetParticlePosition(Position);
  
  G4double timeofspectrum;
  G4double Emean;
  G4double alpha;
  G4double Emean2;
  G4double nu_energy;

  bool retry = true;
  while (retry) {
    retry = false; // Assume that we won't need to retry until proven othe
    //set energy from a tabulated distribution
    //
    timeofspectrum = -99.;
    Emean = -99.;
    Emean2 = -99.;
    alpha = -99.;
    nu_energy = -99.;

    if ((OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy")) == false) {
      timeofspectrum = mSNToolBox.InverseCumulAlgorithm(x_lum, f_lum, a_lum, Fc_lum,nPoints_lum);
      
      G4int timepos = mSNToolBox.findtime(timeofspectrum, mNu_time);
      Emean = mSNToolBox.linealinterpolation(timeofspectrum,mNu_time.at(timepos-1), mNu_time.at(timepos), mNu_meanenergy.at(timepos-1),mNu_meanenergy.at(timepos));
      Emean2 = mSNToolBox.linealinterpolation(timeofspectrum,mNu_time.at(timepos-1), mNu_time.at(timepos), mNu_meanenergysquare.at(timepos-1),mNu_meanenergysquare.at(timepos));
      
      nu_energy = 0;
      G4int count = 0;
      while (nu_energy <= 0.1*MeV) {
        nu_energy = mSNToolBox.EnergyDistribution(Emean, Emean2, alpha);
        count += 1;
        if (count > 10) {
          retry  = true;
          break;
          // To avoid entering in an almost infinite bucle if energy of the chosen time is very unlikely to be higher than 
          // the threshold, go to the beggining and choose a different time of the burst
        }
      }
    } else {
      timeofspectrum = 0.0;
      Emean = mFixedenergy;
      Emean2 = mFixedenergy2;
      nu_energy =  mSNToolBox.InverseCumulAlgorithm(fixFe_X, fixFe_Y, fixFe_a, fixFe_Fc, fixE_nPoints);
    }
  }

  DistFunction(nu_energy);
  
  G4double costheta = mSNToolBox.InverseCumul(angdist_x, angdist_y, angdist_nPoints);
  G4double sintheta = std::sqrt(1. - costheta*costheta);
  G4double phi = twopi*G4UniformRand();
  
  G4double zdir = -costheta; 
  G4double xdir = -sintheta*std::cos(phi);
  G4double ydir = -sintheta*std::sin(phi);
    
  // from nu_energy and costheta, we get e- energy
  G4double e_energy = ElectronEnergy(nu_energy, costheta);
  
  ParticleGun->SetParticleEnergy(e_energy); 
  ParticleGun->SetParticleMomentumDirection(G4ThreeVector(xdir,ydir,zdir));
  
  // now calculate the weight
  G4double sigma = TotalCrossSection(nu_energy);
  G4double Weigh = mSNToolBox.WeighMe(sigma, NTargets);
  
  //sending stuff to analysismanager
  OMSimSNAnalysis &lAnalysisManager = OMSimSNAnalysis::getInstance();
  lAnalysisManager.nuTime = timeofspectrum;
  lAnalysisManager.nuMeanEnergy = Emean;
  lAnalysisManager.nuEnergy = nu_energy;
  lAnalysisManager.cosTheta = costheta;
  lAnalysisManager.primaryEnergy = e_energy; 
  lAnalysisManager.weigh = Weigh;

  //create vertex
  // 
  ParticleGun->GeneratePrimaryVertex(anEvent);
}




void OMSimENES::DistFunction(G4double Enu)
{
  // Angular distribution
  //
  // Carlo Giunti and Chung W.Kim (2007), Fundamentals of Neutrino Physics and Astrophysics, Oxford University Press
  // Chapter 5, eq. 5.29

  angdist_nPoints = 500;
  
  G4double g1=0.73;
  G4double g2=0.23;
  G4double sigma0=2.*pow(Gf,2)*pow(me,2)/pi*pow(197.326e-15,2); 
  
  angdist_x.resize(angdist_nPoints); 
  angdist_y.resize(angdist_nPoints);
  
  G4double min = 0.; G4double max = 1.; 
  G4double delta = (max-min)/G4double(angdist_nPoints-1);
  for (G4int i=0; i<angdist_nPoints; i++) {
    angdist_x[i] = min + i*delta; //costheta
  }
  
  for (G4int j=0; j<angdist_nPoints; j++) {
    G4double dem = pow((pow((me+Enu),2)-pow(Enu,2)*pow(angdist_x[j],2)),2);
    G4double factor1 = 4.*pow(Enu,2)*pow((me+Enu),2)*angdist_x[j]/dem;
    G4double factor2 = 2.*me*Enu*pow(angdist_x[j],2)/dem;
    G4double factor3 = 2.*pow(me,2)*pow(angdist_x[j],2)/dem;
    angdist_y[j] = sigma0*factor1*(pow(g1,2)+pow(g2,2)*pow((1.-factor2),2)-g1*g2*factor3); // dsigma/dcos(theta)
  }
}



G4double OMSimENES::ElectronEnergy(G4double nu_energy, G4double costheta)
{
  // Get electron energy from elastic scattering as a function of incident neutrino energy and scatter angle
  //
  // Carlo Giunti and Chung W.Kim (2007), Fundamentals of Neutrino Physics and Astrophysics, Oxford University Press
  // Chapter 5, eq. 5.27
  G4double energy=2*me*pow(nu_energy,2)*pow(costheta,2)/(pow((me+nu_energy),2)-pow(nu_energy,2)*pow(costheta,2));
  return energy;
}



G4double OMSimENES::TotalCrossSection(G4double energy) {
  // Returns value of the TotalCrossSection for certain energy to use it in WeighMe
  //
  //M. Buchkremer, Electroweak Interactions: Neutral currents in neutrino-lepton elastic
  // scattering experiments, Universit ??e Catholique de Louvain /CP3, 2011.
  G4double sin2thetaw = 0.231;
  G4double sigma = pow(Gf,2)*me*energy/(2.*pi)*(1.+4.*sin2thetaw+16./3.*pow(sin2thetaw,2))*pow(197.326e-15,2)*m*m;
  return sigma;
}

