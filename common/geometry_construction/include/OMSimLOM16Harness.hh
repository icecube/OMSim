
/** @file OMSimLOM16Harness.hh
 *  @brief Construction of the LOM16 harness.
 *  @todo Write documentation
 *  @ingroup common
 */

#pragma once
#include "OMSimDetectorComponent.hh"

class LOM16;
class LOM16Harness : public OMSimDetectorComponent
{
public:
    LOM16Harness(LOM16 *pLOM16);
    void construction();
    G4String mDataKey = "om_LOM16_Harness";

private:
    LOM16 *m_opticalModule;

    void CADHarnessBridges();
    void CADHarnessRopes();
    void CADHarnessPCA();
    void CADHarnessStringConnector();
    void CADString();
    
    void mainDataCable();
    
    const G4double m_ropeRotationAngleX = 11.245557 * deg;
    const G4double m_harnessRotAngle = 30 * deg;
    const G4double m_totalWidth = 170 * mm;
};