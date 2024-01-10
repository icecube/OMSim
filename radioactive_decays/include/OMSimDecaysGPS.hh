/**
 * @file
 * @brief Defines the OMSimDecaysGPS class for the radioactive decays simulation.
 * @ingroup radioactive
 */

#ifndef OMSimDecaysGPS_h
#define OMSimDecaysGPS_h 1
#include "OMSimOpticalModule.hh"
#include <globals.hh>
/**
 * @class OMSimDecaysGPS
 * @brief A class for simulating isotope decays inside the pressure vessel and PMT glass.
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

  /**
   * @brief Set the optical module to be used.
   * @param pOpticalModule Pointer to the optical module.
   */
  void setOpticalModule(OMSimOpticalModule *pOpticalModule) { mOM = pOpticalModule; };
  void setProductionRadius(G4double pProductionRadius);
  G4String getDecayTerminationNuclide();

private:
  // Define a map that maps isotopes to their GPS commands
  std::map<G4String, G4String> mIsotopeCommands = {
      {"U238", "/gps/ion 92 238 0"},
      {"U235", "/gps/ion 92 235 0"},
      {"Ra226", "/gps/ion 88 226 0"},
      {"Ra224", "/gps/ion 88 224 0"},
      {"Th232", "/gps/ion 90 232 0"},
      {"K40", "/gps/ion 19 40 0"}};

  // Define a map that maps isotopes to their termination isotope (Ra is gas state and chains are often not in secular equilibrium)
  // Ra224 with a lifetime of ~4d breaks equilibrium far less than Ra226, with half life ~1600y, so activity of Th232 and Ra224 will probably be very similar
  std::map<G4String, G4String> mTerminationIsotopes = {
      {"U238", "Ra226"},
      {"Th232", "Ra224"},
      {"Ra226", "none"},
      {"Ra224", "none"},
      {"U235", "none"},
      {"K40", "none"}};

  void generalGPS();
  void configureIsotopeGPS(G4String Isotope, G4String location);
  std::map<G4String, G4int> calculateNumberOfDecays(G4MaterialPropertiesTable *pMPT, G4double pTimeWindow, G4double pMass);
  OMSimOpticalModule *mOM;
  G4double mProductionRadius;
  G4String mNuclideStopName;

  OMSimDecaysGPS() = default;
  ~OMSimDecaysGPS() = default;
  OMSimDecaysGPS(const OMSimDecaysGPS &) = delete;
  OMSimDecaysGPS &operator=(const OMSimDecaysGPS &) = delete;
};
#endif
