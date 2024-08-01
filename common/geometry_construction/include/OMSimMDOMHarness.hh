/** @file OMSimMDOMHarness.hh
 *  @brief Construction of mDOM harness.
 *  @ingroup common
 */
#ifndef OMSimMDOMHarness_h
#define OMSimMDOMHarness_h 1

#include "abcDetectorComponent.hh"
#include <G4IntersectionSolid.hh>
#include <G4Cons.hh>

class mDOM;
class mDOMHarness : public abcDetectorComponent
{
public:
    mDOMHarness(mDOM* pMDOM);
    void construction();
    
private:
    mDOM* mOM;
    void bandsAndClamps();
    void bridgeRopesSolid();
    void mainDataCable();
    void pads();
    void PCA();
    void plug();
    void teraBelt();

    const G4double mHarnessRotAngle = 45*deg; //rotation of harness with respect to the module. Valid values are 45, 45+90, 45+180.. etc, otherwise the ropes would go over the PMTs
    const G4double mPlugAngle = 49.0 * deg;
    const G4double mPadThickness = 2.0 * mm;
    const G4double mTeraThickness = 1.0 * mm;
    const G4double mRopeRMax = 3.0 * mm;
    const G4double mBridgeAddedThickness = 14.6 * mm;
    const G4double mRopeRotationAngleX = 11.245557 * deg;
    const G4double mRopeDz = 509.5 * mm;
    const G4double lBridgeCorrection =  7.85*mm * tan(mRopeRotationAngleX);  //  
    const G4double mRopeStartingPoint = mTotalWidth + lBridgeCorrection + mRopeRMax / cos(mRopeRotationAngleX); // this is the actual starting point of the rope, i.e. the distance to the z-axis, which has to be larger than lBridgeROuter[2] in order for the rope not to cut the bridge.

    G4double mTotalWidth;
};

#endif
//
