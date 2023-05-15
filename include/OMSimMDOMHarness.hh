#ifndef OMSimMDOMHarness_h
#define OMSimMDOMHarness_h 1
#include "abcDetectorComponent.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "OMSimInputData.hh"
#include "G4Cons.hh"
#include "G4Tubs.hh"

class mDOM;
class mDOMHarness : public abcDetectorComponent
{
public:
    mDOMHarness(mDOM* pMDOM, OMSimInputData *pData);
    void Construction();
    G4String mDataKey = "mDOM_harness";
    
private:
    mDOM* mOM;
    const G4double mHarnessRotAngle = 45*deg; //rotation of harness with respect to the module. Valid values are 45, 45+90, 45+180.. etc, otherwise the ropes would go over the PMTs
    void GetSharedData();
    void BandsAndClamps();
    void BridgeRopesSolid();
    void MainDataCable();
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
