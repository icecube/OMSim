#ifndef OMSimPMTConstruction_h
#define OMSimPMTConstruction_h 1

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "abcDetectorComponent.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4VisAttributes.hh"
#include "G4SubtractionSolid.hh"
#include "G4PVPlacement.hh"
#include "OMSimInputData.hh"
#include "G4Tubs.hh"
#include <tuple>
#include <map>


class OMSimPMTConstruction : public abcDetectorComponent
{
public:
    OMSimPMTConstruction(InputDataManager* pData);

    void construction();
    G4double GetDistancePMTCenterToPMTtip();
    G4double GetMaxPMTMaxRadius();
    G4VSolid* GetPMTSolid();
    G4LogicalVolume* GetLogicalVolume();
    void placeIt(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4LogicalVolume*& pMother, G4String pNameExtension = "");
    void placeIt(G4Transform3D pTrans, G4LogicalVolume*& pMother, G4String pNameExtension = "");
    void SelectPMT(G4String pPMTtoSelect);
    void SimulateInternalReflections();

    void SimulateHACoating();

private:
    InputDataManager* mData;
    G4String mSelectedPMT;
    G4bool mDynodeSystem = false;
    G4bool mInternalReflections = false;
    G4bool mHACoatingBool = false;
    G4bool mConstructionFinished = false;

    void AppendHACoating();
    std::tuple<G4VSolid*, G4VSolid*> GetBulbSolid(G4String pSide);
    std::tuple<G4VSolid*, G4VSolid*> BulbConstructionSimple(G4String pSide);
    std::tuple<G4VSolid*, G4VSolid*> BulbConstructionFull(G4String pSide);
    G4UnionSolid* SphereEllipsePhotocathode();
    G4UnionSolid* SphereDoubleEllipsePhotocathode(G4String pSide);
    G4UnionSolid* DoubleEllipsePhotocathode(G4String pSide);
    G4UnionSolid* EllipsePhotocathode();
    void checkPhotocathodeThickness();
    void CathodeBackShield(G4LogicalVolume* pPMTIinner);

    void DynodeSystemConstructionCAD(G4LogicalVolume* pMother);
    G4SubtractionSolid* PhotocathodeLayerConstruction();
    void ReadGlobalParameters(G4String pSide);

    G4VSolid* FrontalBulbConstruction(G4String pSide);

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
