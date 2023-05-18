#ifndef OMSimLOM18_h
#define OMSimLOM18_h 1
#include "abcDetectorComponent.hh"
#include "OMSimPMTConstruction.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4IntersectionSolid.hh"
#include "OMSimInputData.hh"
#include "G4Cons.hh"
#include "G4Tubs.hh"
#include "G4Polycone.hh"

extern G4double gRefCone_angle;


class LOM18 : public abcDetectorComponent
{
public:
    LOM18(InputDataManager* pData, G4bool pPlaceHarness = false);
    void construction();
    G4double mCylinderAngle;
    G4double mGlassOutRad;
    G4String mDataKey = "om_LOM18";

  

private:
    OMSimPMTConstruction* mPMTManager;

    //functions
    void getSharedData();


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


    //Shared data from jSON file
    //Vessel specific
    G4double mGlassThickPole;
    G4double mGlassThickEquator;
    G4double mGlassEquatorWidth;
    G4double mGlassPoleLength;

    //PMT specific
    G4double mThetaCenter;
    G4double mThetaEquatorial;
    G4double mEqPMTPhiPhase;

    G4int mNrPolarPMTs;
    G4int mNrCenterPMTs;
    G4int mNrEquatorialPMTs;
    G4int mNrPMTsPerHalf;
    G4int mTotalNrPMTs;

    //gelpad specific
    G4double mPolarPadOpeningAngle;
    G4double mCenterPadOpeningAngle;
    G4double mEqPadOpeningAngle;
    G4double mGelThicknessFrontPolarPMT;
    G4double mGelThicknessFrontCenterPMT;
    G4double mGelThicknessFrontEqPMT;


    //from PMTConstruction class (not readable directly...needs to be changed)
    G4double mTotalLenght;
    G4double mOutRad;
    G4double mSpherePos_y; 
    G4double mEllipsePos_y; 
    G4double mEllipseZaxis;

    //from PMT manager
    G4double mPMToffset;
    G4double mMaxPMTRadius;   

    
    //helper variables
    std::stringstream converter;
    std::stringstream converter2;
    G4Transform3D lTransformers;
    G4RotationMatrix* lRot = new G4RotationMatrix();

    //logical of gelpads
    std::vector<G4LogicalVolume*> mGelPad_logical;
};

#endif
//
