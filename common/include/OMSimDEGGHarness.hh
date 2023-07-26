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
    

    
    //Shared data from jSON file
    G4double mrmin;
    G4double mrmax;
    G4double msphi;
    G4double mdphi;
    G4double mstheta;
    G4double mdtheta;

    const G4double mHarnessRotAngle = 30*deg; //stems from CAD file of DEGG
    G4double mRopeRotationAngleX;
    G4double mRopeDz;
    G4double mTotalWidth;
    G4double mRopeStartingPoint;
};

#endif
//
