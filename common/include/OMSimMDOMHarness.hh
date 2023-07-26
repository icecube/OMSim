#ifndef OMSimMDOMHarness_h
#define OMSimMDOMHarness_h 1

#include "abcDetectorComponent.hh"
#include <G4IntersectionSolid.hh>
#include <G4Cons.hh>

class mDOM;
class mDOMHarness : public abcDetectorComponent
{
public:
    mDOMHarness(mDOM* pMDOM, InputDataManager *pData);
    void construction();
    G4String mDataKey = "mDOM_harness";
    
private:
    mDOM* mOM;
    const G4double mHarnessRotAngle = 45*deg; //rotation of harness with respect to the module. Valid values are 45, 45+90, 45+180.. etc, otherwise the ropes would go over the PMTs
    void getSharedData();
    void BandsAndClamps();
    void BridgeRopesSolid();
    void mainDataCable();
    void Pads();
    void PCA();
    void Plug();
    void TeraBelt();
    
    //Shared data from jSON file
    G4double mPlugAngle;
    G4double mPadThickness;
    G4double mTeraThickness;
    G4double mRopeRMax;
    G4double mBridgeAddedThickness;
    G4double mRopeRotationAngleX;
    G4double mRopeDz;
    G4double mTotalWidth;
    G4double lBridgeCorrection;
    G4double mRopeStartingPoint;
};

#endif
//
