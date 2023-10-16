/** @file OMSimMDOM.hh
 *  @brief Construction of mDOM.
 *  @author Lew Classen, Martin Unland
 *  @ingroup common
 */
#ifndef OMSimMDOM_h
#define OMSimMDOM_h 1

#include "OMSimMDOMFlasher.hh"
#include "OMSimPMTConstruction.hh"
#include "OMSimOpticalModule.hh"

class mDOMHarness;

class mDOM : public OpticalModule
{
private:
    mDOMFlasher *mFlashers;

public:
    mDOM(InputDataManager *pData, G4bool pPlaceHarness = true, G4int pIndex = 0);
    ~mDOM();
    void construction();
    double getPressureVesselWeight() {return 13.0*kg;};
    int getNumberOfPMTs() { return mTotalNrPMTs;};
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
    void setNavigator(G4Navigator *pNavigator) { mFlashers->setNavigator(pNavigator); } // this is needed to get the rotation of the flasher

private:
    
    mDOMHarness *mHarness;
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

    G4double mPMToffset;
    G4double mRefConeIdealInRad;
    const G4double mGlassThick = 13.5*mm; // maximum Glass thickness

public:
    const G4double mCylinderAngle= 2.8*deg; // Deviation angle of cylindrical part of the pressure vessel
    const G4double mGlassOutRad = 176.5*mm; // outer radius of galss cylinder (pressure vessel)
    const G4double mCylHigh = 27.5*mm; // height of cylindrical part of glass half-vessel
    const G4double mGlassInRad = mGlassOutRad - mGlassThick;

private:
    const G4double mGelThicknessFrontPMT = 3.6*mm; // distance between inner glass surface and tip of PMTs
    const G4double mGelThickness = 4.5*mm; // distance between inner glass surface and holding structure, filled with gel
    const G4double mEqPMTrOffset = 2.6*mm; // middle PMT circles are slightly further out due to mEqPMTzOffset
    const G4double mEqPMTzOffset = 10.0*mm; // z-offset of middle PMT circles w.r.t. center of glass sphere
    const G4double mRefConeHalfZ = 15*mm; // half-height of reflector (before cutting to right form)
    const G4double mRefConeSheetThickness = 0.5*mm; // aluminum sheet thickness true for all reflective cones
    const G4double mRefConeToHolder = 1.55*mm; // horizontal distance from K??rcher's construction
    const G4double mThetaPolar = 33.0*deg;
    const G4double mThetaEquatorial = 72.0*deg;
    const G4int mNrPolarPMTs = 4;
    const G4int mNrEqPMTs = 8;
    const G4int mRefConeAngle = 51; 
    const G4double mPolEqPMTPhiPhase = 0*deg;
    const G4int mTotalNrPMTs = (mNrPolarPMTs + mNrEqPMTs) * 2;
    const G4double mSupStructureRad = mGlassOutRad - mGlassThick - mGelThickness;

    const G4double mThetaEqLED = 61 * deg;   // 61 upper sphere, 180-61 lower sphere
    const G4double mThetaPolLED = 8.2 * deg; // 8.2 upper sphere, 180-8.2 lower sphere


};

#endif
//
