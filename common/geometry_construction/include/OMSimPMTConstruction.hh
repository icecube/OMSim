/** @file OMSimPMTConstruction.hh
 *  @brief Construction of the PMTs.
 *
 *  This class creates the solids of the PMTs and place them in the detector/OMs.
 */

#ifndef OMSimPMTConstruction_h
#define OMSimPMTConstruction_h 1
#include "abcDetectorComponent.hh"
#include "OMSimPMTConstruction.hh"
#include "OMSimPMTResponse.hh"
//#include "OMSimDetectorConstruction.hh"

#include <G4UnionSolid.hh>
#include <G4Tubs.hh>
class OMSimDetectorConstruction;


class OMSimPMTConstruction : public abcDetectorComponent
{

public:
    OMSimPMTConstruction(InputDataManager *pData);

    void construction();
    void configureSensitiveVolume(OMSimDetectorConstruction* pDetConst, G4String pName);

    G4double getDistancePMTCenterToTip();
    G4double getMaxPMTRadius();
    G4VSolid *getPMTSolid();
    G4LogicalVolume *getLogicalVolume();
    G4LogicalVolume* getPhotocathodeLV(){return mPhotocathodeLV;};
    double getPMTGlassWeight();

    void placeIt(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4LogicalVolume *&pMother, G4String pNameExtension = "");
    void placeIt(G4Transform3D pTrans, G4LogicalVolume *&pMother, G4String pNameExtension = "");
    void selectPMT(G4String pPMTtoSelect);
    void includeHAcoating();

private:
    G4LogicalVolume* mPhotocathodeLV;
    G4String mSelectedPMT;
    G4bool mDynodeSystem = false;
    G4bool mInternalReflections = false;
    G4bool mHACoatingBool = false;
    G4bool mConstructionFinished = false;

    std::tuple<G4VSolid *, G4VSolid *> getBulbSolid(G4String pSide);
    std::tuple<G4VSolid *, G4VSolid *> simpleBulbConstruction(G4String pSide);
    std::tuple<G4VSolid *, G4VSolid *> fullBulbConstruction(G4String pSide);
    G4VSolid *frontalBulbConstruction(G4String pSide);

    void readGlobalParameters(G4String pSide);

    G4UnionSolid *sphereEllipsePhotocathode();
    G4UnionSolid *sphereDoubleEllipsePhotocathode(G4String pSide);
    G4UnionSolid *doubleEllipsePhotocathode(G4String pSide);
    G4UnionSolid *ellipsePhotocathode();

    void checkPhotocathodeThickness();

    void constructHAcoating();
    void constructCathodeBackshield(G4LogicalVolume *pPMTIinner);
    void constructCADdynodeSystem(G4LogicalVolume *pMother);
    G4SubtractionSolid *constructPhotocathodeLayer();
    
    OMSimPMTResponse* getPMTResponseInstance();

    G4bool mSimpleBulb = false;
    G4double mMissingTubeLength;
    G4PVPlacement *mVacuumBackPhysical;

    bool mCheckOverlaps = true;

    // Variables from json files are saved in the following members
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
