/**
 * @file OMSimAngularScan.hh
 * @brief Defines the AngularScan class for configuring and running GPS beam with angular scans.
 * @ingroup EffectiveArea
 */

#ifndef OMSimBeam_h
#define OMSimBeam_h 1

#include "globals.hh"
#include "TGraph.h"
class Beam
{
public:

  Beam(G4double pBeamRadius, G4double pBeamDistance);
  ~Beam();

  void runErlangenQEBeam();
  void runBeamPicoQuantSetup(double pX, double pY);
  void setWavelength(double pWavelength);
  void configureZCorrection_PicoQuant();
private:


  void configureErlangenQESetup();
  void configureXYZScan_NKTLaser();
  void configureXYZScan_PicoQuantSetup();

  TGraph* mZcorrection;
  G4double mBeamRadius;
  G4double mBeamDistance;
  G4double m_wavelength;
  G4double mTheta;
  G4double mPhi;
};

#endif