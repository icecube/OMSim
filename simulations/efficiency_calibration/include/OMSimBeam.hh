/**
 * @file OMSimAngularScan.hh
 * @brief Defines the AngularScan class for configuring and running GPS beam with angular scans.
 * @ingroup EffectiveArea
 */

#pragma once
#include "globals.hh"
#include "TGraph.h"
class Beam
{
public:

  Beam(G4double pBeamRadius, G4double pBeamDistance);
  ~Beam();

  void runErlangenQEBeam();
  void runBeamPicoQuantSetup(double pX, double pY);
  void runBeamNKTSetup(G4double p_x, G4double p_y);
  void setWavelength(double pWavelength);
  void configureZCorrection_PicoQuant();
private:


  void configureErlangenQESetup();
  void configureXYZScan_NKTLaser();
  void configureXYZScan_PicoQuantSetup();

  TGraph* m_zCorrection;
  G4double m_beamRadius;
  G4double m_beamDistance;
  G4double m_wavelength;
  G4double m_theta;
  G4double m_phi;
};
