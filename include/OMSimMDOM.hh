#ifndef OMSimMDOM_h
#define OMSimMDOM_h 1
#include "abcDetectorComponent.hh"
#include "OMSimMDOMFlasher.hh"
#include "OMSimPMTConstruction.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "G4Cons.hh"
#include "G4Tubs.hh"


class mDOMHarness;

class mDOM : public abcDetectorComponent
{
private:
    mDOMFlasher *mFlashers;

public:
    mDOM(InputDataManager *pData, G4bool pPlaceHarness = true);
    void construction();
    G4double mCylinderAngle;
    G4double mGlassOutRad;
    G4double mCylHigh;
    G4String mDataKey = "om_mDOM";
    G4int mNrTotalLED;
    std::vector<G4Transform3D> mLEDTransformers;           // coordinates from center of the module
    std::vector<std::vector<G4double>> mLED_AngFromSphere; // stores rho (mm),theta (deg),phi (deg) of each LED from the center of its corresponding spherical part. Useful to run the particles.
    
    /**
     * @brief run/beamOn the specified flasher.
     * @details This method triggers the flasher at the given index in the specified module.
     * @param pModuleIndex The index of the module to be flashed (if only one mDOM placed, then 0, otherwise depending on placeIt() order)
     * @param pLEDIndex The index of the flasher within the module.
     */
    void runBeamOnFlasher(G4int pModuleIndex, G4int pLEDIndex) { mFlashers->runBeamOnFlasher(this, pModuleIndex, pLEDIndex); }

private:
    OMSimPMTConstruction *mPMTManager;
    mDOMHarness *mHarness;
    void getSharedData();
    G4SubtractionSolid *equatorialReflector(G4VSolid *pSupportStructure, G4Cons *pReflCone, G4double pAngle, G4String pSuffix);
    void setPMTPositions();
    G4UnionSolid *pressureVessel(const G4double pOutRad, G4String pSuffix);
    G4SubtractionSolid *substractHarnessPlug(G4VSolid *pSolid);
    std::tuple<G4SubtractionSolid *, G4UnionSolid *> supportStructure();
    G4SubtractionSolid *substractFlashers(G4VSolid *lSupStructureSolid);
    void setLEDPositions();

    G4bool mPlaceHarness = true;
    G4bool mHarnessUnion = true; // it should be true for the first module that you build, and then false
    std::vector<G4ThreeVector> mPMTPositions;
    std::vector<G4RotationMatrix> mPMTRotations;
    std::vector<G4RotationMatrix> mPMTRotPhi;
    std::vector<G4ThreeVector> mReflectorPositions;

    // Shared data from jSON file
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
