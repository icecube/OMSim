#ifndef OMSimLOM16_h
#define OMSimLOM16_h 1

#include "OMSimPMTConstruction.hh"

class LOM16 : public abcDetectorComponent
{
public:
    LOM16(InputDataManager* pData, G4bool pPlaceHarness = false);
    ~LOM16();
    void construction();
    G4String mDataKey = "om_LOM16";

  

private:
    OMSimPMTConstruction* mPMTManager;

    //functions
    void getSharedData();
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


    //Shared data from jSON file
    G4double mGlassOutRad;
    G4int mNrPolarPMTs;
    G4int mNrEqPMTs;
    G4int mTotalNrPMTs;

    //helper variables
    std::stringstream mConverter;
    std::stringstream mConverter2;

    //for gelpads
    G4double mGelPadDZ;

    //logical of gelpads
    std::vector<G4LogicalVolume*> mGelPad_logical;


};

#endif
//
