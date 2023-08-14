#ifndef OMSimDecaysGPS_h
#define OMSimDecaysGPS_h 1
#include "abcDetectorComponent.hh"
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

private:
  // Define a map that maps isotopes to their commands
  std::map<G4String, G4String> mIsotopeCommands = {
      {"U238", "/gps/ion 92 238 0"},
      {"U235", "/gps/ion 92 235 0"},
      {"Th232", "/gps/ion 90 232 0"},
      {"K40", "/gps/ion 19 40 0"}
  };
  void common_GPS_commands(); 
  void pressure_vessel_isotope_GPS(G4String pIsotope);
  void PMT_isotope_GPS(G4String pIsotope, G4int pPMTNr);
  void simulate_decays_in_time_window();
  std::map<G4String, G4int> calculate_number_of_decays(G4MaterialPropertiesTable* pMPT, G4double pTimeWindow, G4double pMass);
  OpticalModule* mOM;
  G4double mProductionRadius;
};
#endif




