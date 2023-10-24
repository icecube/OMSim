/**
 * @file
 * @brief Defines the IsotopeDecays class for the radioactive decays simulation.
 * @ingroup radioactive
 */

#ifndef OMSimDecaysGPS_h
#define OMSimDecaysGPS_h 1
#include "OMSimOpticalModule.hh"
#include <globals.hh>
/**
 * @class IsotopeDecays
 * @brief A class for simulating isotope decays inside the pressure vessel and PMT glass.
 * @ingroup radioactive
 */
class IsotopeDecays
{
public:
  IsotopeDecays(G4double pProductionRadius);
  ~IsotopeDecays();
  void simulateDecaysInOpticalModule(G4double pTimeWindow);

  /**
   * @brief Set the optical module to be used.
   * @param pOpticalModule Pointer to the optical module.
   */
  void setOpticalModule(OMSimOpticalModule* pOpticalModule){mOM = pOpticalModule;};

private:

  // Define a map that maps isotopes to their commands
  std::map<G4String, G4String> mIsotopeCommands = {
      {"U238", "/gps/ion 92 238 0"},
      {"U235", "/gps/ion 92 235 0"},
      {"Th232", "/gps/ion 90 232 0"},
      {"K40", "/gps/ion 19 40 0"}
  };
  void generalGPS(); 
  void configureIsotopeGPS(G4String Isotope, G4String location, G4int optParam = -999);
  std::map<G4String, G4int> calculateNumberOfDecays(G4MaterialPropertiesTable* pMPT, G4double pTimeWindow, G4double pMass);
  OMSimOpticalModule* mOM;
  G4double mProductionRadius;
};
#endif




