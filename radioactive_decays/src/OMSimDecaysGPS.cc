#include "OMSimDecaysGPS.hh"
#include "OMSimUIinterface.hh"
#include <G4SystemOfUnits.hh>
#include <G4Poisson.hh>

IsotopeDecays::IsotopeDecays(G4double pProductionRadius)
{
    mProductionRadius = pProductionRadius;
}

/**
 * @brief Configures common GPS commands for the radioactive decays.
 */
void IsotopeDecays::common_GPS_commands()
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    lUIinterface.applyCommand("/control/verbose  0");
    lUIinterface.applyCommand("/run/verbose 0");
    lUIinterface.applyCommand("/event/verbose 0");
    lUIinterface.applyCommand("/tracking/verbose 0");
    lUIinterface.applyCommand("/run/initialize");
    lUIinterface.applyCommand("/process/em/fluo true");
    lUIinterface.applyCommand("/process/em/auger true");
    lUIinterface.applyCommand("/process/em/pixe true");
    lUIinterface.applyCommand("/gps/particle ion");
    lUIinterface.applyCommand("/gps/pos/centre 0 0 0 m");
    lUIinterface.applyCommand("/gps/ene/mono 0 eV");
    lUIinterface.applyCommand("/gps/pos/type Volume");
    lUIinterface.applyCommand("/gps/pos/shape Sphere");
    lUIinterface.applyCommand("/gps/pos/radius ", mProductionRadius, " mm");
    lUIinterface.applyCommand("/gps/ang/type iso");
}

/**
 * @brief Configures GPS for the production and decay of an isotope within the pressure vessel of an OM.
 *
 * If the daughter of the configured isotope is unstable, it will also decay in the same position as its mother
 *
 * @param Isotope The isotope which is going to be produced.
 */
void IsotopeDecays::pressure_vessel_isotope_GPS(G4String Isotope)
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    common_GPS_commands();

    if (mIsotopeCommands.find(Isotope) != mIsotopeCommands.end())
    {
        lUIinterface.applyCommand(mIsotopeCommands[Isotope]);
    }
    lUIinterface.applyCommand("/gps/pos/confine PressureVessel");
}

/**
 * @brief Configures GPS for isotope decays in PMT glass of an specific PMT.
 *
 * If the daughter of the configured isotope is unstable, it will also decay in the same position as its mother
 *
 * @param Isotope The isotope which is going to be produced.
 * @param PMTNr Number of PMT where the decays are simulated
 */
void IsotopeDecays::PMT_isotope_GPS(G4String Isotope, G4int PMTNr)
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    common_GPS_commands();

    if (mIsotopeCommands.find(Isotope) != mIsotopeCommands.end())
    {
        lUIinterface.applyCommand(mIsotopeCommands[Isotope]);
    }
    lUIinterface.applyCommand("/gps/pos/confine PMT_", PMTNr);
}

std::map<G4String, G4int> IsotopeDecays::calculate_number_of_decays(G4MaterialPropertiesTable *pMPT, G4double pTimeWindow, G4double pMass)
{
    std::map<G4String, G4int> mNumberDecays;
    for (auto &pair : mIsotopeCommands)
    {
        G4String lIsotope = pair.first;
        G4double lActivity = pMPT->GetConstProperty(lIsotope + "_ACTIVITY");
        mNumberDecays[pair.first] = G4int(G4Poisson(lActivity * pTimeWindow * pMass));
    }
    return mNumberDecays;
}

void IsotopeDecays::simulate_decays_in_time_window()
{
    OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();

    G4double lTimeWindow = lArgs.get<G4double>("time_window");

    if (lArgs.get<bool>("PV_decays"))
    {
        G4double lMass = mOM->get_pressure_vessel_weight();
        G4LogicalVolume *lPVVolume = mOM->getComponent("PressureVessel").VLogical;
        G4MaterialPropertiesTable *lMPT = lPVVolume->GetMaterial()->GetMaterialPropertiesTable();
        std::map<G4String, G4int> lNumberDecays = calculate_number_of_decays(lMPT, lTimeWindow, lMass);

        for (auto &pair : lNumberDecays)
        {
            G4String lIsotope = pair.first;
            G4int lNrDecays = pair.second;
            pressure_vessel_isotope_GPS(lIsotope);
            lUIinterface.runBeamOn(lNrDecays);
        }
    }

    if (lArgs.get<bool>("PMT_decays"))
    {
        G4double lMass = mOM->get_PMT_manager()->get_PMT_glass_weight();
        G4LogicalVolume *lPVVolume = mOM->get_PMT_manager()->getComponent("PMT").VLogical;
        G4MaterialPropertiesTable *lMPT = lPVVolume->GetMaterial()->GetMaterialPropertiesTable();

        for (int pmt = 0; pmt < (int)mOM->get_number_of_PMTs(); pmt++)
        {
            std::map<G4String, G4int> lNumberDecays = calculate_number_of_decays(lMPT, lTimeWindow, lMass);

            for (auto &pair : lNumberDecays)
            {
                G4String lIsotope = pair.first;
                G4int lNrDecays = pair.second;
                PMT_isotope_GPS(lIsotope, pmt);
                lUIinterface.runBeamOn(lNrDecays);
            }
        }
    }
}

