#ifndef OMSimSimpleGPSBeams_h
#define OMSimSimpleGPSBeams_h 1

#include "globals.hh"

class AngularScan
{
public:
  AngularScan(G4double pBeamRadius, G4double pBeamDistance, G4double pWavelength);
  ~AngularScan();
  void configureScan();
  void runSingleAngularScan(G4double pPhi, G4double pTheta);

private:
    void ConfigurePosCoordinates();
    void ConfigureAngCoordinates();
    G4double mBeamRadius;
    G4double mBeamDistance;
    G4double mWavelength;
    G4double mTheta;
    G4double mPhi;
};

#endif