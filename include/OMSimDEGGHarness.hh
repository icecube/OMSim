#ifndef OMSimDEGGHarness_h
#define OMSimDEGGHarness_h 1
#include "abcDetectorComponent.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "OMSimInputData.hh"
#include "G4Cons.hh"
#include "G4Tubs.hh"

class DEgg;
class dEGGHarness : public abcDetectorComponent
{
public:
    //dEGGHarness(OMSimInputData *pData);
    dEGGHarness(DEgg* pDEGG, OMSimInputData *pData);
    void Construction();
    G4String mDataKey = "om_DEGG_Harness";

private:
    DEgg* mOM;

    void PlaceCADHarness();
    void PlaceCADPenetrator();
    void MainDataCable();
    G4VSolid*Build(G4double rmin,G4double rmax,G4double sphi,G4double dphi,G4double stheta,G4double dtheta);
    void GetSharedData();
    

    
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
