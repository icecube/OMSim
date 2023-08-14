#include "OMSimAngularScan.hh"
#include "OMSimUIinterface.hh"
#include <G4SystemOfUnits.hh>

/**
 * @param pBeamRadius The radius of the beam.
 * @param pBeamDistance The distance of the beam from the origin.
 * @param pWavelength The wavelength of the photons to be generated.
 */
AngularScan::AngularScan(G4double pBeamRadius, G4double pBeamDistance, G4double pWavelength)
{
    mBeamRadius = pBeamRadius;
    mBeamDistance = pBeamDistance;
    mWavelength = pWavelength;
}
/**
 * @brief Configures the position coordinates of the beam based on the polar and azimuthal angles.
 */
void AngularScan::configurePosCoordinates()
{
    double lRho = mBeamDistance * sin(mTheta);
    double lPosX = lRho * cos(mPhi);
    double lPosY = lRho * sin(mPhi);
    double lPosZ = mBeamDistance * cos(mTheta);
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    lUIinterface.applyCommand("/gps/pos/centre", lPosX, lPosY, lPosZ, "mm");
    lUIinterface.applyCommand("/gps/pos/radius", mBeamRadius, "mm");
}
/**
 * @brief Configures the angular coordinates of the beam based on the polar and azimuthal angles.
 */
void AngularScan::configureAngCoordinates()
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    double x, y, z;
    x = -sin(mPhi);
    y = cos(mPhi);
    z = 0;
    lUIinterface.applyCommand("/gps/pos/rot1", x, y, z);
    lUIinterface.applyCommand("/gps/ang/rot1", x, y, z);

    x = -cos(mPhi) * cos(mTheta);
    y = -sin(mPhi) * cos(mTheta);
    z = sin(mTheta);

    lUIinterface.applyCommand("/gps/pos/rot2", x, y, z);
    lUIinterface.applyCommand("/gps/ang/rot2", x, y, z);
}

/**
 * @brief Configure the GPS settings, such as the particle properties, beam position and direction.
 */
void AngularScan::configureScan()
{
    // Obtain an instance of OMSimUIinterface
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();

    // Use applyCommand to send commands
    lUIinterface.applyCommand("/event/verbose 0");
    lUIinterface.applyCommand("/control/verbose 0");
    lUIinterface.applyCommand("/run/verbose 0");
    lUIinterface.applyCommand("/gps/particle opticalphoton");
    lUIinterface.applyCommand("/gps/energy", 1239.84193 / mWavelength, "eV");
    lUIinterface.applyCommand("/gps/pos/type Plane");
    lUIinterface.applyCommand("/gps/pos/shape Circle");
    lUIinterface.applyCommand("/gps/pos/centre 0 0 30 cm");
    lUIinterface.applyCommand("/gps/pos/radius 80 mm");
    lUIinterface.applyCommand("/gps/pos/rot1 0 1 0");
    lUIinterface.applyCommand("/gps/pos/rot2 0 0 1");
    lUIinterface.applyCommand("/gps/ang/rot1 0 1 0");
    lUIinterface.applyCommand("/gps/ang/rot2 0 0 1");
    lUIinterface.applyCommand("/gps/ang/type beam2d");
    lUIinterface.applyCommand("/gps/ang/sigma_x 0");
    lUIinterface.applyCommand("/gps/ang/sigma_y 0");
    configurePosCoordinates();
    configureAngCoordinates();
}

/**
 * @brief Run a single angular scan with the specified angles.
 * @param pPhi The azimuthal angle in degrees.
 * @param pTheta The polar angle in degrees.
 */
void AngularScan::runSingleAngularScan(G4double pPhi, G4double pTheta)
{
    mTheta = pTheta * deg;
    mPhi = pPhi * deg;
    configureScan();
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    lUIinterface.runBeamOn();
}