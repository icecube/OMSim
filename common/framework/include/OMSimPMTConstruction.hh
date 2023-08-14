#ifndef OMSimPMTConstruction_h
#define OMSimPMTConstruction_h 1


#include "abcDetectorComponent.hh"

#include <G4UnionSolid.hh>
#include <G4Tubs.hh>


class OMSimPMTConstruction : public abcDetectorComponent
{
public:
    OMSimPMTConstruction(InputDataManager* pData);

    void construction();
    G4double getDistancePMTCenterToTip();
    G4double getMaxPMTRadius();
    G4VSolid* getPMTSolid();
    G4LogicalVolume* getLogicalVolume();
    double getPMTGlassWeight();

    void placeIt(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4LogicalVolume*& pMother, G4String pNameExtension = "");
    void placeIt(G4Transform3D pTrans, G4LogicalVolume*& pMother, G4String pNameExtension = "");
    void selectPMT(G4String pPMTtoSelect);

    void includeHAcoating();

private:
    InputDataManager* mData;
    G4String mSelectedPMT;
    G4bool mDynodeSystem = false;
    G4bool mInternalReflections = false;
    G4bool mHACoatingBool = false;
    G4bool mConstructionFinished = false;

    void constructHAcoating();
    std::tuple<G4VSolid*, G4VSolid*> getBulbSolid(G4String pSide);
    std::tuple<G4VSolid*, G4VSolid*> simpleBulbConstruction(G4String pSide);
    std::tuple<G4VSolid*, G4VSolid*> fullBulbConstruction(G4String pSide);
    G4UnionSolid* sphereEllipsePhotocathode();
    G4UnionSolid* sphereDoubleEllipsePhotocathode(G4String pSide);
    G4UnionSolid* doubleEllipsePhotocathode(G4String pSide);
    G4UnionSolid* ellipsePhotocathode();

    void checkPhotocathodeThickness();
    void constructCathodeBackshield(G4LogicalVolume* pPMTIinner);

    void constructCADdynodeSystem(G4LogicalVolume* pMother);
    G4SubtractionSolid* constructPhotocathodeLayer();
    void readGlobalParameters(G4String pSide);

    G4VSolid* frontalBulbConstruction(G4String pSide);

    G4bool mSimpleBulb = false;
    G4double mMissingTubeLength;
    G4PVPlacement* mVacuumBackPhysical;


    bool mCheckOverlaps = true;

    //Variables from json files are saved in the following members
    G4double mTotalLenght;
    G4double mTubeWidth;
    G4double mOutRad;
    G4double mEllipseXYaxis;
    G4double mEllipseZaxis;
    G4double mSphereEllipseTransition_r;
    G4double mSpherePos_y;
    G4double mEllipsePos_y;
};

#endif
//
