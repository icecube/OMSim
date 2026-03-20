/**
 * @file
 * @brief Defines the OMSimDecaysGPS class for the radioactive decays simulation.
 * @ingroup radioactive
 */
/*
#pragma once
#include "OMSimOpticalModule.hh"
#include <globals.hh>
*/
/**
 * @class OMSimDecaysGPS
 * @brief A class for simulating isotope decays inside the pressure vessel and PMT glass.
 * @ingroup radioactive
 */
/*
class OMSimDecaysGPS
{
public:

  static OMSimDecaysGPS &getInstance()
  {
    static OMSimDecaysGPS instance;
    return instance;
  }

  void simulateDecaysInPMTs(G4double pTimeWindow);
  void simulateDecaysInPressureVessel(G4double pTimeWindow);
*/
  /**
   * @brief Set the optical module to be used.
   * @param p_opticalModule Pointer to the optical module.
   */
  /*
  void setOpticalModule(OMSimOpticalModule *p_opticalModule) { m_opticalModule = p_opticalModule; };
  void setProductionRadius(G4double pProductionRadius);
  G4String getDecayTerminationNuclide();
  G4ThreeVector sampleNextDecayPosition(G4ThreeVector p_currentPosition);

private:
  // Define a map that maps isotopes to their GPS commands
  std::map<G4String, G4String> m_isotopeCommands = {
      {"U238", "/gps/ion 92 238 0"},
      {"U235", "/gps/ion 92 235 0"},
      {"Ra226", "/gps/ion 88 226 0"},
      {"Ra224", "/gps/ion 88 224 0"},
      {"Th232", "/gps/ion 90 232 0"},
      {"K40", "/gps/ion 19 40 0"}};

  // Define a map that maps isotopes to their termination isotope (Ra is gas state and chains are often not in secular equilibrium)
  // Ra224 with a lifetime of ~4d breaks equilibrium far less than Ra226, with half life ~1600y, so activity of Th232 and Ra224 will probably be very similar
  std::map<G4String, G4String> m_terminationIsotopes = {
      {"U238", "Ra226"},
      {"Th232", "Ra224"},
      {"Ra226", "none"},
      {"Ra224", "none"},
      {"U235", "none"},
      {"K40", "none"}};

  void generalGPS();
  void configureIsotopeGPS(G4String Isotope, G4String location);
  std::map<G4String, G4int> calculateNumberOfDecays(G4MaterialPropertiesTable *pMPT, G4double pTimeWindow, G4double pMass);
  OMSimOpticalModule *m_opticalModule;
  G4double m_productionRadius;
  G4String m_nuclideStopName;

  OMSimDecaysGPS() = default;
  ~OMSimDecaysGPS() = default;
  OMSimDecaysGPS(const OMSimDecaysGPS &) = delete;
  OMSimDecaysGPS &operator=(const OMSimDecaysGPS &) = delete;
};
*/










// -----------------------------------------------------------------
/**
 * @file
 * @brief Defines the OMSimDecaysGPS class for radioactive decays and muon scans.
 * @ingroup radioactive
 */

#pragma once

#include "OMSimOpticalModule.hh"
#include <globals.hh>
#include <map>
#include <G4ThreeVector.hh>

/**
 * @class OMSimDecaysGPS
 * @brief A class for simulating isotope decays and muon scans in the optical module.
 * @ingroup radioactive
 */
class OMSimDecaysGPS
{
public:
    static OMSimDecaysGPS &getInstance()
    {
        static OMSimDecaysGPS instance;
        return instance;
    }

    void simulateDecaysInPMTs(G4double pTimeWindow);
    void simulateDecaysInPressureVessel(G4double pTimeWindow);

    void setOpticalModule(OMSimOpticalModule *p_opticalModule) { m_opticalModule = p_opticalModule; }
    void setProductionRadius(G4double pProductionRadius);
    G4String getDecayTerminationNuclide();
    G4ThreeVector sampleNextDecayPosition(G4ThreeVector p_currentPosition);

    // ---------- Muon scan methods ----------
    void setMuonScanParameters();
    void runSingleAngularScan(G4double pPhi, G4double pTheta);
    G4double getBeamRadius()   const { return m_beamRadius; }
    G4double getBeamDistance() const { return m_beamDistance; }
    void configureScan();
private:
    OMSimDecaysGPS() = default;
    ~OMSimDecaysGPS() = default;
    OMSimDecaysGPS(const OMSimDecaysGPS &) = delete;
    OMSimDecaysGPS &operator=(const OMSimDecaysGPS &) = delete;

    // Internal helpers
    void generalGPS();
    void configureIsotopeGPS(G4String Isotope, G4String location);
    std::map<G4String, G4int> calculateNumberOfDecays(G4MaterialPropertiesTable *pMPT, G4double pTimeWindow, G4double pMass);

    // Muon scan helpers
   // void configureScan();
    void configurePosCoordinates();
    void configureAngCoordinates();

    OMSimOpticalModule *m_opticalModule = nullptr;

    G4double m_productionRadius = 0;
    G4String m_nuclideStopName;

    // Decay-related
    std::map<G4String, G4String> m_isotopeCommands = {
        {"U238", "/gps/ion 92 238 0"},
        {"U235", "/gps/ion 92 235 0"},
        {"Ra226", "/gps/ion 88 226 0"},
        {"Ra224", "/gps/ion 88 224 0"},
        {"Th232", "/gps/ion 90 232 0"},
        {"K40", "/gps/ion 19 40 0"}};

    std::map<G4String, G4String> m_terminationIsotopes = {
        {"U238", "Ra226"},
        {"Th232", "Ra224"},
        {"Ra226", "none"},
        {"Ra224", "none"},
        {"U235", "none"},
        {"K40", "none"}};

    // Muon scan parameters
    G4double m_beamRadius = 0;
    G4double m_beamDistance = 0;
    G4double m_wavelength = 0;
    G4double m_theta = 0;
    G4double m_phi = 0;
};
