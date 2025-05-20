
/** @file OMSimLOM18Harness.hh
 *  @brief Construction of the LOM18 harness.
 *  @ingroup common
 */

#pragma once
#include "OMSimDetectorComponent.hh"

class LOM18;
class LOM18Harness : public OMSimDetectorComponent
{
public:
    LOM18Harness(LOM18 *pLOM18);
    void construction();
    G4String mDataKey = "om_LOM18_Harness";

private:
    LOM18 *m_opticalModule;

    void CADHarnessWaistband();
    void CADHarnessRopes();
    void CADHarnessPCA();
    void CADString();
    
    void mainDataCable();
    
    const G4double m_harnessRotAngle = 0 * deg;
    const G4double m_totalWidth = 170 * mm; //Verify this value!
};