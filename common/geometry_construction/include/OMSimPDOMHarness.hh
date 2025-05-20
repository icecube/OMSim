
/** @file OMSimDOMHarness.hh
 *  @brief Construction of the DOM harness.
 *  @ingroup common
 */

#pragma once
#include "OMSimDetectorComponent.hh"

class DOM;
class DOMHarness : public OMSimDetectorComponent
{
public:
    DOMHarness(DOM *pDOM);
    void construction();
    G4String mDataKey = "om_DOM_Harness";

private:
    DOM *m_opticalModule;

    void CADHarnessWaistband();
    void CADHarnessRopes();
    void CADHarnessPCA();
    void PlaceCADString();

    void mainDataCable();
    void buildHarnessSolid();

    const G4double m_rMin = 150.0 * mm;
    const G4double m_rMax = 155.0 * mm;
    const G4double m_sPhi = 0.0 * deg;
    const G4double m_dPhi = 6.283185307;   // This is already in radians
    const G4double m_sTheta = 1.383031327; // This is in radians
    const G4double m_dTheta = 0.37553;     // This is in radians
    const G4double m_ropeRotationAngleX = 11.245557 * deg;
    const G4double m_harnessRotAngle = 30 * deg;
    const G4double m_totalWidth = 185 * mm; //Verify this value!
};

