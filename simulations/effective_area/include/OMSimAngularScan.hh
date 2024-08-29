/**
 * @file OMSimAngularScan.hh
 * @brief Defines the AngularScan class for configuring and running GPS beam with angular scans.
 * @ingroup EffectiveArea
 */

#pragma once

#include "globals.hh"
/**
 * @class AngularScan
 * @brief Class for defining and running simple GPS beam configurations with angular scans.
 *
 * The AngularScan class provides functionalities to configure and run simple GPS beam configurations
 * with angular scans. It allows setting the beam radius, beam distance, and wavelength of the photons
 * to be generated. It also supports specifying the angle of incidence with respect to the target
 * and performs the simulation for each angular configuration.
 * @ingroup EffectiveArea
 */
class AngularScan
{
public:

  AngularScan(G4double pBeamRadius, G4double pBeamDistance, G4double pWavelength);
  ~AngularScan();


  void configureScan();
  void runSingleAngularScan(G4double pPhi, G4double pTheta);

private:

  void configurePosCoordinates();
  void configureAngCoordinates();
  G4double m_beamRadius;
  G4double m_beamDistance;
  G4double m_wavelength;
  G4double m_theta;
  G4double m_phi;
};

