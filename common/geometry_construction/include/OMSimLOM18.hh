/** @file OMSimLOM18.hh
 *  @brief Construction of LOM18.
 *
 *  @author Javi Vara & Markus Dittmer
 *  @ingroup common
 */
#ifndef OMSimLOM18_h
#define OMSimLOM18_h 1

#include "OMSimPMTConstruction.hh"
#include "OMSimOpticalModule.hh"

#include <G4LogicalVolume.hh>
#include <G4Polycone.hh>

class LOM18 : public OpticalModule
{
public:
    LOM18(InputDataManager* pData, G4bool pPlaceHarness = false);
    ~LOM18();
    void construction();
    double getPressureVesselWeight() {return 17.0*kg;};
    int getNumberOfPMTs() { return mTotalNrPMTs;};
    
private:

    G4Polycone* createLOM18OuterSolid();
    G4Polycone* createLOM18InnerSolid();

    void appendEquatorBand();
    void placeCADSupportStructure(G4LogicalVolume* lInnerVolumeLogical);
    G4LogicalVolume* mSupportStructureLogical;
    void placeCADPenetrator(G4LogicalVolume* lInnerVolumeLogical);
    
    
    void setPMTPositions();
    void createGelpadLogicalVolumes(G4Polycone* lGelSolid);

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




    //from PMTConstruction class (not readable directly...needs to be changed)
    G4double mTotalLenght;
    G4double mOutRad;
    G4double mSpherePos_y; 
    G4double mEllipsePos_y; 
    G4double mEllipseZaxis;

    //helper variables
    std::stringstream mConv;
    std::stringstream converter2;
    G4Transform3D lTransformers;
    G4RotationMatrix* lRot = new G4RotationMatrix();

    //logical of gelpads
    std::vector<G4LogicalVolume*> mGelPad_logical;

    G4double mGlassEquatorWidth = 159*mm;
    G4double mGlassPoleLength = 270*mm;
    G4double mGlassThickPole = 12.5*mm;
    G4double mGlassThickEquator = 16.5*mm;
    
    G4double mThetaCenter = 48.0*deg;
    G4double mThetaEquatorial = 60.0*deg;
    G4int mNrPolarPMTs = 1;
    G4int mNrCenterPMTs = 4;
    G4int mNrEquatorialPMTs = 4;
    G4double mEqPMTPhiPhase = 45.0*deg;

    //gelpad specific
    G4double mPolarPadOpeningAngle = 30.0*deg;
    G4double mCenterPadOpeningAngle = 10.0*deg;
    G4double mEqPadOpeningAngle = 5.0*deg;
    G4double mGelThicknessFrontPolarPMT = 3.5*mm;
    G4double mGelThicknessFrontCenterPMT = 12.93*mm;
    G4double mGelThicknessFrontEqPMT = 14.52*mm;

    G4int mNrPMTsPerHalf = mNrPolarPMTs + mNrCenterPMTs + mNrEquatorialPMTs;
    G4int mTotalNrPMTs = (mNrPolarPMTs + mNrCenterPMTs + mNrEquatorialPMTs) * 2;
    //from PMT manager
    G4double mPMToffset;
    G4double mMaxPMTRadius;   

    public:
    G4double mCylinderAngle = 1.5*deg;
    G4double mGlassOutRad;
};

#endif
//
