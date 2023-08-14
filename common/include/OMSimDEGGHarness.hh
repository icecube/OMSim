#ifndef OMSimDEGGHarness_h
#define OMSimDEGGHarness_h 1
#include "abcDetectorComponent.hh"


class DEGG;
class DEggHarness : public abcDetectorComponent
{
public:
    //DEggHarness(InputDataManager *pData);
    DEggHarness(DEGG* pDEGG, InputDataManager *pData);
    void construction();
    G4String mDataKey = "om_DEGG_Harness";

private:
    DEGG* mOM;

    void placeCADHarness();
    void placeCADPenetrator();
    void mainDataCable();
    G4VSolid*buildHarnessSolid(G4double rmin,G4double rmax,G4double sphi,G4double dphi,G4double stheta,G4double dtheta);
    void getSharedData();

    const G4double mRmin = 150.0 * mm;
    const G4double mRmax = 155.0 * mm;
    const G4double mSphi = 0.0 * deg;
    const G4double mDphi = 6.283185307; // This is already in radians
    const G4double mStheta = 1.383031327; // This is in radians
    const G4double mDtheta = 0.37553; // This is in radians
    const G4double mRopeRotationAngleX = 11.245557 * deg;
    const G4double mHarnessRotAngle = 30*deg; 
    const G4double mTotalWidth = 170 * mm; 
};

#endif
//
