/**
 * @file OMSimAngularScan.hh
 * @brief Defines the AngularScan class for configuring and running GPS beam with angular scans.
 * @ingroup EffectiveArea
 */

#ifndef OMSimAngularScan_h
#define OMSimAngularScan_h 1

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
  /**
   * @param pBeamRadius The radius of the beam.
   * @param pBeamDistance The distance of the beam from the origin.
   * @param pWavelength The wavelength of the photons to be generated.
   */
  AngularScan(G4double pBeamRadius, G4double pBeamDistance, G4double pWavelength);
  ~AngularScan();

  /**
   * @brief Configure the GPS settings, such as the particle properties, beam position and direction.
   */
  void configureScan();

  /**
   * @brief Run a single angular scan with the specified angles.
   * @param pPhi The azimuthal angle in degrees.
   * @param pTheta The polar angle in degrees.
   */
  void runSingleAngularScan(G4double pPhi, G4double pTheta);

private:
  /**
   * @brief Configures the position coordinates of the beam based on the polar and azimuthal angles.
   */
  void configurePosCoordinates();

  /**
   * @brief Configures the angular coordinates of the beam based on the polar and azimuthal angles.
   */
  void configureAngCoordinates();
  G4double mBeamRadius;
  G4double mBeamDistance;
  G4double mWavelength;
  G4double mTheta;
  G4double mPhi;
};

#endif