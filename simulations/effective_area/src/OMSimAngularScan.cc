#include "OMSimAngularScan.hh"
#include "OMSimUIinterface.hh"
#include <G4SystemOfUnits.hh>
#include "G4VisExecutive.hh"
#include "G4RunManager.hh"
#include <cmath>

/**
 * @param p_beamRadius The radius of the beam.
 * @param p_beamDistance The distance of the beam from the origin.
 * @param p_wavelength The wavelength of the photons to be generated.
 */

AngularScan::AngularScan(G4double p_beamRadius, G4double p_beamDistance, G4double p_wavelength)
    : m_beamRadius(p_beamRadius), m_beamDistance(p_beamDistance), m_wavelength(p_wavelength)
{
}

/**
 * @brief Configures the position coordinates of the beam based on the polar and azimuthal angles.
 */

void AngularScan::configurePosCoordinates()
{
    double rho = m_beamDistance * sin(m_theta);
    double x = rho * cos(m_phi);
    double y = rho * sin(m_phi);
    double z = m_beamDistance * cos(m_theta);

    OMSimUIinterface &uiInterface = OMSimUIinterface::getInstance();
    uiInterface.applyCommand("/gps/pos/centre", x, y, z, "mm");
    uiInterface.applyCommand("/gps/pos/radius", m_beamRadius, "mm");
}

/**
 * @brief Configures the angular coordinates of the beam based on the polar and azimuthal angles.
 */

void AngularScan::configureAngCoordinates()
{
    OMSimUIinterface &uiInterface = OMSimUIinterface::getInstance();
    double x, y, z;

    x = -sin(m_phi);
    y = cos(m_phi);
    z = 0;
    uiInterface.applyCommand("/gps/pos/rot1", x, y, z);
    uiInterface.applyCommand("/gps/ang/rot1", x, y, z);

    x = -cos(m_phi) * cos(m_theta);
    y = -sin(m_phi) * cos(m_theta);
    z = sin(m_theta);
    uiInterface.applyCommand("/gps/pos/rot2", x, y, z);
    uiInterface.applyCommand("/gps/ang/rot2", x, y, z);
}

/**
 * @brief Configure the GPS settings, such as the particle properties, beam position and direction.
 */

void AngularScan::configureScan()
{
    // simulate full pool coverage
    double poolWidth = 346.8 * cm;
    double poolHeight = 168.6 * cm;
    double poolDiagonal = std::sqrt(std::pow(poolWidth, 2) + std::pow(poolHeight, 2));
    m_beamRadius = poolDiagonal / 2 + 10 * cm;
    double beamDistance = 200 * cm;

    OMSimUIinterface &uiInterface = OMSimUIinterface::getInstance();
    OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();

    uiInterface.applyCommand("/event/verbose 0");
    uiInterface.applyCommand("/control/verbose 0");
    uiInterface.applyCommand("/run/verbose 0");

    uiInterface.applyCommand("/gps/particle mu+");
    //uiInterface.applyCommand("/gps/energy 4e9 eV");
   // uiInterface.applyCommand("/gps/particle opticalphoton");
   // uiInterface.applyCommand("/gps/energy", 1239.84193 / m_wavelength, "eV");

    uiInterface.applyCommand("/gps/pos/type Plane");
    uiInterface.applyCommand("/gps/pos/shape Circle");
    uiInterface.applyCommand("/gps/pos/radius", m_beamRadius, "mm");
    uiInterface.applyCommand("/gps/pos/centre 0 0", beamDistance, "mm");

    uiInterface.applyCommand("/gps/pos/rot1 0 1 0");
    uiInterface.applyCommand("/gps/pos/rot2 0 0 1");
    uiInterface.applyCommand("/gps/ang/rot1 0 1 0");
    uiInterface.applyCommand("/gps/ang/rot2 0 0 1");
    uiInterface.applyCommand("/gps/ang/type beam2d");
    uiInterface.applyCommand("/gps/ang/sigma_x 0");
    uiInterface.applyCommand("/gps/ang/sigma_y 0");

    if (args.get<bool>("scint_off")) {
        log_debug("Inactivating scintillation process");
        uiInterface.applyCommand("/process/inactivate Scintillation");
    }

    configurePosCoordinates();
    configureAngCoordinates();
}

/**
 * @brief Run a single angular scan with the specified angles.
 * @param p_phi The azimuthal angle in degrees.
 * @param p_theta The polar angle in degrees.
 */

void AngularScan::runSingleAngularScan(G4double p_phi, G4double p_theta)
{
    m_theta = p_theta * deg;
    m_phi = p_phi * deg;
   // m_theta=90*deg;
   // m_phi=180*deg;

    configureScan();

    OMSimUIinterface &uiInterface = OMSimUIinterface::getInstance();
    uiInterface.runBeamOn(1); // (1) for muon flux
}
