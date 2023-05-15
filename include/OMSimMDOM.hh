#ifndef OMSimMDOM_h
#define OMSimMDOM_h 1
#include "abcDetectorComponent.hh"
#include "OMSimPMTConstruction.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"
//#include "OMSimInputData.hh"
#include "G4Cons.hh"
#include "G4Tubs.hh"

extern G4double gRefCone_angle;

class mDOMHarness;

class mDOM : public abcDetectorComponent
{
public:
    mDOM(OMSimInputData* pData, G4bool pPlaceHarness = true);
    void Construction();
    G4double mCylinderAngle;
    G4double mGlassOutRad;
    G4double mCylHigh;
    G4String mDataKey = "om_mDOM";
    G4int mNrTotalLED;
    std::vector<G4Transform3D> mLEDTransformers; //coordinates from center of the module
    std::vector<std::vector<G4double>> mLED_AngFromSphere; //stores rho (mm),theta (deg),phi (deg) of each LED from the center of its corresponding spherical part. Useful to run the particles.

private:
    OMSimPMTConstruction* mPMTManager;
    mDOMHarness* mHarness;
    void GetSharedData();
    G4SubtractionSolid* EquatorialReflector(G4VSolid* pSupportStructure, G4Cons* pReflCone, G4double pAngle, G4String pSuffix);
    void SetPMTPositions();
    G4UnionSolid* PressureVessel(const G4double pOutRad, G4String pSuffix);
    G4SubtractionSolid* SubstractHarnessPlug(G4VSolid* pSolid);
    std::tuple<G4SubtractionSolid*, G4UnionSolid*> SupportStructure();
    std::tuple<G4SubtractionSolid*, G4UnionSolid*, G4UnionSolid*, G4Tubs*>  LedFlashers(G4VSolid* lSupStructureSolid);
    void SetLEDPositions();


    G4bool mPlaceHarness = true;
    G4bool mHarnessUnion = true; //it should be true for the first module that you build, and then false
    std::vector<G4ThreeVector> mPMTPositions;
    std::vector<G4RotationMatrix> mPMTRotations;
    std::vector<G4RotationMatrix> mPMTRotPhi;
    std::vector<G4ThreeVector> mReflectorPositions;


    //Shared data from jSON file
    G4double mGlassThick;
    G4double mGelThicknessFrontPMT;
    G4double mGelThickness;
    G4double mEqPMTrOffset;
    G4double mEqPMTzOffset;
    G4double mRefConeHalfZ;
    G4double mRefConeSheetThickness;
    G4double mThetaPolar;
    G4double mThetaEquatorial;
    G4int mNrPolarPMTs;
    G4int mNrEqPMTs;
    G4double mGlassInRad;
    G4double mRefConeAngle;
    G4int mTotalNrPMTs;
    G4double mPMToffset;
    G4double mRefConeIdealInRad;
    G4double mSupStructureRad;

};

#endif
//
