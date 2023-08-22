#include "OMSimSNTools.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4VTouchable.hh"
#include <stdlib.h>
#include "G4Navigator.hh"
#include "G4TouchableHistoryHandle.hh"

extern double gworldsize;
extern  G4double gRadius;
extern  G4double gHeight;
extern G4bool gfixmeanenergy;
extern G4Navigator* aNavigator;
extern G4double	gmdomseparation;
extern G4int	gn_mDOMs;

OMSimSNTools::OMSimSNTools(){
}

OMSimSNTools::~OMSimSNTools(){
}

bool OMSimSNTools::CheckVolumeFormDOMs(G4ThreeVector position){
    aNavigator->LocateGlobalPointAndSetup(position);
    G4TouchableHistoryHandle aTouchable = aNavigator->CreateTouchableHistoryHandle();
    G4int HistoryDepth = aTouchable->GetHistoryDepth();
    if (HistoryDepth > 0) {return true;}
    return false;
}

G4ThreeVector OMSimSNTools::RandomPosition() {
  //maximum lenght of generation cylinder "Rmax"
  //Note that this is the distance from the center of the cylinder to the corner of the circumscribed rectangle around the cylinder
  G4double Rmax = pow(3,1./2.)*gworldsize*m; 
  G4double Rmax2 = gworldsize*m; //radius of generation cylinder
  
  G4double posz;
  G4double posx;
  G4double posy;
  G4ThreeVector Position;
  G4double R3; //distance from center
  G4double R2; //distance wrt z axis
  R3 = 0*m;
  R2 = 0*m;
  G4bool boolparameter = true;
  while ( ( boolparameter==true ) || (R3 >= Rmax) || (R2 >= Rmax2)) {
    G4double posornegX = 1;
    if (G4UniformRand()<0.5) { posornegX = -1;}
            G4double posornegY = 1;
    if (G4UniformRand()<0.5) { posornegY = -1;}
            G4double posornegZ = 1;
    if (G4UniformRand()<0.5) { posornegZ = -1;}
    posz = posornegZ*(G4UniformRand()*gHeight);
    posx = posornegX*(G4UniformRand()*gRadius);
    posy = posornegY*(G4UniformRand()*gRadius);
    Position = G4ThreeVector(posx,posy,posz);
    R3 = pow(pow(Position[0],2)+pow(Position[1],2)+pow(Position[2],2),1./2.);
    R2 = pow(pow(Position[0],2)+pow(Position[1],2),1./2.);
    boolparameter = CheckVolumeFormDOMs(Position);
  } 
  return Position;
}


G4double OMSimSNTools::linealinterpolation(G4double x, G4double x1, G4double x2, G4double y1, G4double y2) {
    G4double slope = (y2 - y1) / (x2 - x1);
    G4double result = (slope * (x - x1) + y1);
    return result;
}


G4double OMSimSNTools::EnergyDistribution(G4double Emean, G4double Emean2, G4double& alpha)
{   
  G4int nPoints1 = 500;
  if (gfixmeanenergy == false) {
	  alpha = GetAlpha(Emean, Emean2);
  } 
  std::vector<G4double> x1;
  std::vector<G4double> f1;
  MakeEnergyDistribution(Emean, alpha, nPoints1, x1 , f1);
	G4double choosenenergy = InverseCumul(x1, f1, nPoints1);
	return choosenenergy;
}

G4double OMSimSNTools::GetAlpha(G4double Emean,G4double Emean2)
{
	G4double alpha = (2*pow(Emean,2)-Emean2)/(Emean2-pow(Emean,2));
	return alpha;
}

G4double OMSimSNTools::InverseCumul(std::vector<G4double>  xvals, std::vector<G4double>  yvals, G4int nPoints)
{
  std::vector<G4double>  x_g;
  std::vector<G4double>  f_g;           //f(x)
  std::vector<G4double>  a_g;           //slopes
  std::vector<G4double>  Fc_g;          //cumulative of f
  GetSlopes(xvals, yvals, nPoints, x_g, f_g, a_g, Fc_g);
  G4double x_rndm =  InverseCumulAlgorithm(x_g, f_g, a_g, Fc_g, nPoints);
  return x_rndm;
}


G4int OMSimSNTools::findtime(G4double time, std::vector<G4double> timearray)
{
  for (unsigned int j=0; j<timearray.size(); j++) {
    if (time <= timearray.at(j)) {
      return j;
    };
  };
 G4cerr << "FATAL ERROR -> Not posible to find time of spectrum!!!" << G4endl;
 return 0;
}
   
G4double OMSimSNTools::InverseCumulAlgorithm(std::vector<G4double>  x, std::vector<G4double>  f, std::vector<G4double>  a, std::vector<G4double>  Fc, G4int  nPoints)
{
  //choose y randomly
  G4double y_rndm = G4UniformRand()*Fc[nPoints-1];
  //find bin
  G4int j = nPoints-2;
  while ((Fc[j] > y_rndm) && (j > 0)) j--;
  //y_rndm --> x_rndm :  Fc(x) is second order polynomial
  G4double x_rndm = x[j];
  G4double aa = a[j];
  if (aa != 0.) {
    G4double b = f[j]/aa, c = 2*(y_rndm - Fc[j])/aa;
    G4double delta = b*b + c;
    G4int sign = 1; if (aa < 0.) sign = -1;
    x_rndm += sign*std::sqrt(delta) - b;    
  } else if (f[j] > 0.) {
    x_rndm += (y_rndm - Fc[j])/f[j];
  };
  return x_rndm;
}



void GetSlopes(std::vector<G4double>  xvals, std::vector<G4double>  yvals, G4int nPoints, std::vector<G4double>&  x, std::vector<G4double>&  f, std::vector<G4double>&  a, std::vector<G4double>&  Fc)
{
  // create a copy of the array. Really not necessary...
  x.resize(nPoints); f.resize(nPoints);
  for (G4int j=0; j<nPoints; j++) {
    x[j] = xvals.at(j); f[j] = yvals.at(j);
  };
  //compute slopes
  //
  a.resize(nPoints);
  for (G4int j=0; j<nPoints-1; j++) { 
    a[j] = (f[j+1] - f[j])/(x[j+1] - x[j]);
  };
  //compute cumulative function
  //
  Fc.resize(nPoints);  
  Fc[0] = 0.;
  for (G4int j=1; j<nPoints; j++) {
    Fc[j] = Fc[j-1] + 0.5*(f[j] + f[j-1])*(x[j] - x[j-1]);
  };     
}


G4double NumberOfTargets(G4int targetPerMolecule) {
  G4double Density = 921.6*kg/m3; //Density of ice at -50 celsius degrees
  G4double MolarMass = 18.01528e-3*kg; //kg per mol
  G4double Na = 6.022140857e23;
  G4double Nm = Density/MolarMass*Na;//molecules/m^3 of ice
  G4double Nt = Nm*targetPerMolecule;
  return Nt;
}


void MakeEnergyDistribution(G4double Emean, G4double alpha, G4int nPoints, std::vector<G4double>& x, std::vector<G4double>& f)
{
  G4double min = 0.; G4double max = 80.; 
  G4double delta = (max-min)/G4double(nPoints-1);
  x.resize(nPoints); f.resize(nPoints);

  for (G4int i=0; i<nPoints; i++) {
    x[i] = (min + i*delta)*MeV; //Energy
  }

  for (G4int j=0; j<nPoints; j++) {
  f[j] = pow(x[j],alpha)*exp(-(alpha+1.)*x[j]/Emean); // F(e), energy dist. function
  }
}


G4double OMSimSNTools::WeighMe(G4double sigma, G4double NTargets) {
  double weigh = sigma*NTargets*(2*gHeight);
  return weigh;
}


