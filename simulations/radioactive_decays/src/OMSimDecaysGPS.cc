#include "OMSimDecaysGPS.hh"
#include "OMSimLogger.hh"
#include "OMSimUIinterface.hh"
#include <G4SystemOfUnits.hh>
#include <G4Poisson.hh>

void OMSimDecaysGPS::setProductionRadius(G4double p_productionRadius)
{
    m_productionRadius = p_productionRadius;
}

G4String OMSimDecaysGPS::getDecayTerminationNuclide()
{
    return m_nuclideStopName;
}
/**
 * @brief Configures common GPS commands for the radioactive decays.
 */
void OMSimDecaysGPS::generalGPS()
{
    log_trace("Configuring general GPS");
    OMSimUIinterface &ui = OMSimUIinterface::getInstance();
    OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();

    ui.applyCommand("/control/verbose  0");
    ui.applyCommand("/run/verbose 0");
    ui.applyCommand("/event/verbose 0");
    ui.applyCommand("/tracking/verbose 0");
    ui.applyCommand("/run/initialize");
    ui.applyCommand("/process/em/fluo true");
    ui.applyCommand("/process/em/auger true");
    ui.applyCommand("/process/em/pixe true");
    ui.applyCommand("/gps/particle ion");
    G4ThreeVector position = m_opticalModule->m_placedPositions.at(0);
    ui.applyCommand("/gps/pos/centre ", position.x()/m, " ", position.y()/m, " ", position.z()/m, " m");
    ui.applyCommand("/gps/ene/mono 0 eV");
    ui.applyCommand("/gps/pos/type Volume");
    ui.applyCommand("/gps/pos/shape Sphere");
    ui.applyCommand("/gps/pos/radius ", m_productionRadius, " mm");
    ui.applyCommand("/gps/ang/type iso");

    if (args.get<bool>("scint_off"))
    {
        log_trace("Inactivating scintillation process");
        ui.applyCommand("/process/inactivate Scintillation");
    }

    if (args.get<bool>("cherenkov_off"))
    {
        log_trace("Inactivating Cerenkov process");
        ui.applyCommand("/process/inactivate Cerenkov");
    }
}

/**
 * @brief Configures GPS for the production and decay of an isotope within a specified location.
 *
 * If the daughter of the configured isotope is unstable, it will also decay in the same position as its mother
 *
 * @param p_isotope The isotope which is going to be produced.
 * @param p_volumeName The volume name where the isotope decays.
 */
void OMSimDecaysGPS::configureIsotopeGPS(G4String p_isotope, G4String p_volumeName)
{
    log_trace("Configuring GPS for isotope {} in volume {}", p_isotope, p_volumeName);
    OMSimUIinterface &ui = OMSimUIinterface::getInstance();

    generalGPS();

    if (m_isotopeCommands.find(p_isotope) != m_isotopeCommands.end())
    {
        ui.applyCommand(m_isotopeCommands[p_isotope]);
    }
    ui.applyCommand("/gps/pos/confine " + p_volumeName);
}

/**
 * @brief Calculates the number of decays for isotopes.
 * @param p_MPT Pointer to the material properties table.
 * @param p_timeWindow Time window for decays.
 * @param p_mass Mass of the volume in which decays occur.
 * @return Map of isotopes and their respective number of decays.
 */
std::map<G4String, G4int> OMSimDecaysGPS::calculateNumberOfDecays(G4MaterialPropertiesTable *p_MPT, G4double p_timeWindow, G4double p_mass)
{
    std::map<G4String, G4int> numberDecays;
    log_trace("Calculating number of decays from material isotope activity...");
    for (auto &pair : m_isotopeCommands)
    {
        G4String isotope = pair.first;
        G4double activity = p_MPT->GetConstProperty(isotope + "_ACTIVITY");
        numberDecays[pair.first] = G4int(G4Poisson(activity * p_timeWindow * p_mass));
        log_trace("Number of calculated decays for Isotope {} is {}", isotope, numberDecays[pair.first]);
    }
    return numberDecays;
}

/**
 * @brief Simulates the decays in the pressure vessel of the optical module.
 * @param p_timeWindow The livetime that should be simulated.
 */
void OMSimDecaysGPS::simulateDecaysInPressureVessel(G4double p_timeWindow)
{
    log_trace("Simulating radioactive decays in pressure vessel in a time window of {} seconds", p_timeWindow);

    OMSimUIinterface &ui = OMSimUIinterface::getInstance();

    G4double mass = m_opticalModule->getPressureVesselWeight();
    G4String pressureVesselName = "PressureVessel_" + std::to_string(m_opticalModule->m_index);

    G4LogicalVolume *pressureVesselLogicalVolume = m_opticalModule->getComponent(pressureVesselName).VLogical;
    G4MaterialPropertiesTable *MPT = pressureVesselLogicalVolume->GetMaterial()->GetMaterialPropertiesTable();

    std::map<G4String, G4int> numberDecays = calculateNumberOfDecays(MPT, p_timeWindow, mass);

    for (auto &pair : numberDecays)
    {
        G4String isotope = pair.first;
        G4int decayCount = pair.second;
        m_nuclideStopName = m_terminationIsotopes[pair.first];
        configureIsotopeGPS(isotope, pressureVesselName);

        log_trace("Simulating {} decays of {} in pressure vessel", decayCount, isotope);
        ui.runBeamOn(decayCount);
    }
}

/**
 * @brief Simulates the decays in the PMTs of the optical module.
 * @param p_timeWindow The livetime that should be simulated.
 */
void OMSimDecaysGPS::simulateDecaysInPMTs(G4double p_timeWindow)
{
    log_trace("Simulating radioactive decays in glass of PMTs in a time window of {} seconds", p_timeWindow);
    OMSimUIinterface &ui = OMSimUIinterface::getInstance();
    G4double mass = m_opticalModule->getPMTmanager()->getPMTGlassWeight();
    G4LogicalVolume *pressureVesselLogicalVolume = m_opticalModule->getPMTmanager()->getLogicalVolume();
    G4MaterialPropertiesTable *MPT = pressureVesselLogicalVolume->GetMaterial()->GetMaterialPropertiesTable();

    for (int pmt = 0; pmt < (int)m_opticalModule->getNumberOfPMTs(); pmt++)
    {
        std::map<G4String, G4int> numberDecays = calculateNumberOfDecays(MPT, p_timeWindow, mass);

        for (auto &pair : numberDecays)
        {
            G4String isotope = pair.first;
            G4int decayCount = pair.second;
            m_nuclideStopName = m_terminationIsotopes[pair.first];

            configureIsotopeGPS(isotope, "PMT"+std::to_string(pmt));

            log_trace("Simulating {} decays of {} in PMT {}", decayCount, isotope, pmt);
            ui.runBeamOn(decayCount);
        }
    }
}
