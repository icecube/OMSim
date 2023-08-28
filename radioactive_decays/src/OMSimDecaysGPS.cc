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
void IsotopeDecays::generalGPS()
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
 * @brief Configures GPS for the production and decay of an isotope within a specified location.
 *
 * If the daughter of the configured isotope is unstable, it will also decay in the same position as its mother
 *
 * @param Isotope The isotope which is going to be produced.
 * @param pVolumeName The volume name where the isotope decays.
 * @param optParam An optional parameter related to the location. For "PMT", it could be the PMT number.
 */
void IsotopeDecays::configureIsotopeGPS(G4String Isotope, G4String pVolumeName, G4int optParam)
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



std::map<G4String, G4int> IsotopeDecays::calculateNumberOfDecays(G4MaterialPropertiesTable *pMPT, G4double pTimeWindow, G4double pMass)
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

void IsotopeDecays::simulateDecaysInOpticalModule(G4double pTimeWindow)
{
    OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();

    if (!lArgs.get<bool>("no_PV_decays"))
    {
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
/*
    if (!lArgs.get<bool>("no_PMT_decays"))
    {
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
*/
}

