
/** @file OMSimDEGGHarness.hh
 *  @brief Construction of the DEGG harness.
 *  @todo Write documentation
 *  @ingroup common
 */

#pragma once
#include "OMSimDetectorComponent.hh"

class DEGG;
class DEggHarness : public OMSimDetectorComponent
{
public:
    DEggHarness(DEGG *pDEGG);
    void construction();
    G4String mDataKey = "om_DEGG_Harness";

private:
    DEGG *m_opticalModule;

    void placeCADHarness();
    void placeCADPenetrator();
    void mainDataCable();
    G4VSolid *buildHarnessSolid(G4double rmin, G4double rmax, G4double sphi, G4double dphi, G4double stheta, G4double dtheta);

    const G4double m_rMin = 150.0 * mm;
    const G4double m_rMax = 155.0 * mm;
    const G4double m_sPhi = 0.0 * deg;
    const G4double m_dPhi = 6.283185307;   // This is already in radians
    const G4double m_sTheta = 1.383031327; // This is in radians
    const G4double m_dTheta = 0.37553;     // This is in radians
    const G4double m_ropeRotationAngleX = 11.245557 * deg;
    const G4double m_harnessRotAngle = 30 * deg;
    const G4double m_totalWidth = 170 * mm;
};

