/** @file OMSimMDOMHarness.hh
 *  @brief Construction of mDOM harness.
 *  @ingroup common
 */
#pragma once

#include "OMSimDetectorComponent.hh"
#include <G4IntersectionSolid.hh>
#include <G4Cons.hh>

class mDOM;
class mDOMHarness : public OMSimDetectorComponent
{
public:
    mDOMHarness(mDOM* pMDOM);
    void construction();
    
private:
    mDOM* m_opticalModule;
    void bandsAndClamps();
    void bridgeRopesSolid();
    void mainDataCable();
    void pads();
    void PCA();
    void plug();
    void teraBelt();

    const G4double m_harnessRotAngle = 45*deg; //rotation of harness with respect to the module. Valid values are 45, 45+90, 45+180.. etc, otherwise the ropes would go over the PMTs
    const G4double m_plugAngle = 49.0 * deg;
    const G4double m_padThickness = 2.0 * mm;
    const G4double m_teraThickness = 1.0 * mm;
    const G4double m_ropeRMax = 3.0 * mm;
    const G4double m_bridgeAddedThickness = 14.6 * mm;
    const G4double m_ropeRotationAngleX = 11.*deg;
    const G4double m_ropeDz = 509.5 * mm;
    const G4double m_bridgeCorrection =  7.85*mm * tan(m_ropeRotationAngleX);  // 

    G4double m_totalWidth;
    G4double m_ropeStartingPoint;
};
