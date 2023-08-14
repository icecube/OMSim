#ifndef OMSimLOM16_h
#define OMSimLOM16_h 1

#include "OMSimPMTConstruction.hh"
#include "OMSimOpticalModule.hh"

class LOM16 : public OpticalModule
{
public:
    LOM16(InputDataManager* pData, G4bool pPlaceHarness = false);
    ~LOM16();
    void construction();
    double get_pressure_vessel_weight() {return 5.38+5.35;};
    int get_number_of_PMTs() { return mTotalNrPMTs;};
    
private:
    G4UnionSolid* pressureVessel(const G4double pOutRad, G4String pSuffix);

    //Lom specific functions
    void placeCADSupportStructure();

    void appendEquatorBand();
    
    //for gelpad and PMT creation
    void placePMTsAndGelpads(G4VSolid* lGelSolid,G4LogicalVolume* lGelLogical);
    void setPMTAndGelpadPositions();
    void createGelpadLogicalVolumes(G4VSolid* lGelSolid);
    void placePMTs(G4LogicalVolume* lInnerVolumeLogical);
    void placeGelpads(G4LogicalVolume* lInnerVolumeLogical);

    //selection variables
    G4bool mPlaceHarness = true;
    G4bool mHarnessUnion = true; //it should be true for the first module that you build, and then false

    //vectors for positions and rotations
    std::vector<G4ThreeVector> mPMTPositions;
    std::vector<G4ThreeVector> mGelpadPositions;
    std::vector<G4double> mPMT_theta;
    std::vector<G4double> mPMT_phi;

    //helper variables
    std::stringstream mConverter;
    std::stringstream mConverter2;

    //logical of gelpads
    std::vector<G4LogicalVolume*> mGelPad_logical;

    G4String mPMTModel =  "pmt_Hamamatsu_4inch";

    const G4double mInternalCAD_x = 68.248*mm;
    const G4double mInternalCAD_y = 0*mm;
    const G4double mInternalCAD_z = -124.218*mm;
    const G4double mGelPadDZ = 30.0*mm;
    const G4double mGlassOutRad = 153.2*mm;
    const G4double mGlassThick = 12.0*mm;
    const G4double mGlassInRad = mGlassOutRad - mGlassThick;
    const G4double mCylHigh = 68.8*mm;
    const G4double mCylinderAngle = 2.5*deg;
    const G4double mGelThicknessFrontPMT = 3.6*mm;
    const G4double mGelThickness = 4.5*mm;
    const G4double mEqPMTrOffset = 2.6*mm;
    const G4double mEqPMTzOffset = 62.5*mm;
    const G4double mRefConeHalfZ = 15*mm;
    const G4double mRefConeSheetThickness = 0.5*mm;
    const G4double mRefConeToHolder = 1.55*mm;
    const G4double mThetaPolar = 36.0*deg;
    const G4double mThetaEquatorial = 62.0*deg;
    const G4int mNrPolarPMTs = 4;
    const G4int mNrEqPMTs = 4;
    const G4double mPolEqPMTPhiPhase = 45.0*deg;
    const G4double mEqTiltAngle = 15.0*deg;
    const G4double mPolPadOpeningAngle = 30.0*deg;
    const G4double mEqPadOpeningAngle = 22.0*deg;
    const G4int mTotalNrPMTs = (mNrPolarPMTs + mNrEqPMTs) * 2;

    G4double mPMToffset;
    G4double mMaxPMTRadius;

    const G4double mEqBandWidth = 45 * mm; //Total width (both halves)
    const G4double mEqBandThickness = 1 * mm; //Thickness since its a 3D object
};

#endif
//
