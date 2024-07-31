/**
 * @file OMSimAngularScan.hh
 * @brief Defines the AngularScan class for configuring and running GPS beam with angular scans.
 * @ingroup EffectiveArea
 */

#ifndef OMSimBeam_h
#define OMSimBeam_h 1

#include "globals.hh"

class Beam
{
public:

  Beam(G4double pBeamRadius, G4double pBeamDistance);
  ~Beam();

  void runBeam(G4double pPhi, G4double pTheta);
  void runBeam(double pX, double pY, double pZ);
  void setWavelength(double pWavelength);
private:

  void configurePosCoordinates();
  void configureAngCoordinates();
  void configureAngScan();
  void configureXYZScan(double pX, double pY, double pZ);
  void setXYZ(double x, double y, double z);
  G4double mBeamRadius;
  G4double mBeamDistance;
  G4double mWavelength;
  G4double mTheta;
  G4double mPhi;
};

#endif