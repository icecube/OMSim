#include "OMSimDecaysGPS.hh"
#include "OMSimUIinterface.hh"
#include <G4SystemOfUnits.hh>
#include <G4Poisson.hh>

void OMSimDecaysGPS::setProductionRadius(G4double pProductionRadius)
{
    mProductionRadius = pProductionRadius;
}

G4String OMSimDecaysGPS::getDecayTerminationNuclide()
{
    return mNuclideStopName;
}
/**
 * @brief Configures common GPS commands for the radioactive decays.
 */
void OMSimDecaysGPS::generalGPS()
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();

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

    if(lArgs.get<bool>("scint_off")) lUIinterface.applyCommand("/process/inactivate Scintillation");
    if(lArgs.get<bool>("cherenkov_off")) lUIinterface.applyCommand("/process/inactivate Cerenkov");
}

/**
 * @brief Configures GPS for the production and decay of an isotope within a specified location.
 *
 * If the daughter of the configured isotope is unstable, it will also decay in the same position as its mother
 *
 * @param Isotope The isotope which is going to be produced.
 * @param pVolumeName The volume name where the isotope decays.
 * @param optParam An optional parameter related to the location. For "PMT", use e.g. the PMT number.
 */
void OMSimDecaysGPS::configureIsotopeGPS(G4String Isotope, G4String pVolumeName, G4int optParam)
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    generalGPS();

    if (mIsotopeCommands.find(Isotope) != mIsotopeCommands.end())
    {
        lUIinterface.applyCommand(mIsotopeCommands[Isotope]);
    }
    
    if(optParam != -999)
    {
        lUIinterface.applyCommand("/gps/pos/confine " + pVolumeName, optParam);
    }
    else
    {
        lUIinterface.applyCommand("/gps/pos/confine " + pVolumeName);
    }
}


/**
 * @brief Calculates the number of decays for isotopes.
 * @param pMPT Pointer to the material properties table.
 * @param pTimeWindow Time window for decays.
 * @param pMass Mass of the volume in which decays occur.
 * @return Map of isotopes and their respective number of decays.
 */
std::map<G4String, G4int> OMSimDecaysGPS::calculateNumberOfDecays(G4MaterialPropertiesTable *pMPT, G4double pTimeWindow, G4double pMass)
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



/**
 * @brief Simulates the decays in the pressure vessel of the optical module.
 * @param pTimeWindow The livetime that should be simulated.
 */
void OMSimDecaysGPS::simulateDecaysInPressureVessel(G4double pTimeWindow)
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();

    G4double lMass = mOM->getPressureVesselWeight();
    G4LogicalVolume *lPVVolume = mOM->getComponent("PressureVessel").VLogical;
    G4MaterialPropertiesTable *lMPT = lPVVolume->GetMaterial()->GetMaterialPropertiesTable();
    std::map<G4String, G4int> lNumberDecays = calculateNumberOfDecays(lMPT, pTimeWindow, lMass);

    for (auto &pair : lNumberDecays)
    {
        G4String lIsotope = pair.first;
        G4int lNrDecays = pair.second;
        configureIsotopeGPS(lIsotope, "PressureVessel");
        lUIinterface.runBeamOn(lNrDecays);
    }
}

/**
 * @brief Simulates the decays in the PMTs of the optical module.
 * @param pTimeWindow The livetime that should be simulated.
 */
void OMSimDecaysGPS::simulateDecaysInPMTs(G4double pTimeWindow)
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();

    G4double lMass = mOM->getPMTmanager()->getPMTGlassWeight();
    G4LogicalVolume *lPVVolume = mOM->getPMTmanager()->getLogicalVolume();
    G4MaterialPropertiesTable *lMPT = lPVVolume->GetMaterial()->GetMaterialPropertiesTable();

    for (int pmt = 0; pmt < (int)mOM->getNumberOfPMTs(); pmt++)
    {
        std::map<G4String, G4int> lNumberDecays = calculateNumberOfDecays(lMPT, pTimeWindow, lMass);

        for (auto &pair : lNumberDecays)
        {
            G4String lIsotope = pair.first;
            G4int lNrDecays = pair.second;
            configureIsotopeGPS(lIsotope, "PMT", pmt);
            lUIinterface.runBeamOn(lNrDecays);
        }
    }

}
