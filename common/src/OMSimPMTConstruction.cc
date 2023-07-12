/** @file OMSimPMTConstruction.cc
 *  @brief Construction of the PMTs.
 *
 *  This class creates the solids of the PMTs and place them in the detector/OMs.
 *  Methods are order as in the header. Please take a look there first!
 *  @author Lew Classen, Martin Unland
 *  @date October 2021
 *
 *  @version Geant4 10.7
 *
 *  @todo
 *
 */
#include "OMSimPMTConstruction.hh"
#include <dirent.h>
#include "abcDetectorComponent.hh"

#include <G4Box.hh>
#include <G4Cons.hh>
#include <G4Ellipsoid.hh>
#include "G4LogicalVolume.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include <G4PVPlacement.hh>
#include <G4Sphere.hh>
#include <G4SubtractionSolid.hh>
#include "G4SystemOfUnits.hh"
#include <G4Torus.hh>
#include <G4Tubs.hh>
#include <G4UnionSolid.hh>
#include <G4UnitsTable.hh>
#include "G4VisAttributes.hh"
#include "OMSimLogger.hh"
#include "CADMesh.hh" 
#include "OMSimCommandArgsTable.hh"


/**
 * Constructor of the class. The InputData instance has to be passed here in order to avoid loading the input data twice and redifining the same materials.
 * @param pData OMSimInputData instance
 */
OMSimPMTConstruction::OMSimPMTConstruction(InputDataManager* pData)
{
    mData = pData;
}

/**
 * Constructs the PMT Solid.
 */
void OMSimPMTConstruction::construction()
{
    mComponents.clear();
    G4VSolid* lPMTSolid;
    G4VSolid* lVacuumPhotocathodeSolid;
    G4VSolid* lGlassInside;
    std::tie(lPMTSolid, lVacuumPhotocathodeSolid) = GetBulbSolid("jOuterShape");
    std::tie(lGlassInside, lVacuumPhotocathodeSolid) = GetBulbSolid("jInnerShape");
    G4SubtractionSolid* lVacuumBack = new G4SubtractionSolid("Vacuum Tube solid", lGlassInside, lVacuumPhotocathodeSolid, 0, G4ThreeVector(0, 0, 0));


    //Logicals
    G4LogicalVolume* lPMTlogical = new G4LogicalVolume(lPMTSolid, mData->getMaterial("RiAbs_Glass_Tube"), "PMT tube logical");
    //G4LogicalVolume* lPMTlogical = new G4LogicalVolume(lPMTSolid, mData->GetMaterial("Ri_Glass_Tube"), "PMT tube logical");
    G4LogicalVolume* lTubeVacuum = new G4LogicalVolume(lGlassInside, mData->getMaterial("Ri_Vacuum"), "PMTvacuum");

    G4SubtractionSolid* lVacuumPhotocathodeSolidNew = PhotocathodeLayerConstruction();

    G4LogicalVolume* lPhotocathode = new G4LogicalVolume(lVacuumPhotocathodeSolidNew, mData->getMaterial("RiAbs_Photocathode"), "Photocathode");
    G4LogicalVolume* lVacuumBackLogical = new G4LogicalVolume(lVacuumBack, mData->getMaterial("Ri_Vacuum"), "PMTvacuum");
    

    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lPhotocathode, "Photocathode", lTubeVacuum, false, 0, mCheckOverlaps);
    mVacuumBackPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lVacuumBackLogical, "VacuumTubeBack", lTubeVacuum, false, 0, mCheckOverlaps);
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lTubeVacuum, "VacuumTube", lPMTlogical, false, 0, mCheckOverlaps);
    
    appendComponent(lPMTSolid, lPMTlogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PMT");

    if (mHACoatingBool) AppendHACoating();
    //CathodeBackShield(lVacuumBackLogical);
    DynodeSystemConstructionCAD(lVacuumBackLogical);


    lPhotocathode->SetVisAttributes(mPhotocathodeVis);
    lPMTlogical->SetVisAttributes(mGlassVis);
    lTubeVacuum->SetVisAttributes(mInvisibleVis);
    lVacuumBackLogical->SetVisAttributes(mAirVis);
    mConstructionFinished = true;
}


void OMSimPMTConstruction::AppendHACoating()
{
    ReadGlobalParameters("jOuterShape");
    //G4double lVisualCorr = 0.0*mm;
    //if (gVisual) lVisualCorr = 0.01*mm;
    G4Tubs* lHACoatingUncut = new G4Tubs("HACoatingUncut", 0, 0.5 * mTubeWidth + 0.1 * mm, mMissingTubeLength - 0.1 * mm, 0, 2 * CLHEP::pi);
    G4SubtractionSolid* lHACoatingCut = new G4SubtractionSolid("Bulb tube solid", lHACoatingUncut, mComponents.at("PMT").VSolid, 0, G4ThreeVector(0, 0, mMissingTubeLength));
    G4LogicalVolume* lHACoatingLogical = new G4LogicalVolume(lHACoatingCut, mData->getMaterial("NoOptic_Absorber"), "HACoating");
    lHACoatingLogical->SetVisAttributes(mAbsorberSemiTransparentVis);
    appendComponent(lHACoatingCut, lHACoatingLogical, G4ThreeVector(0, 0, -mMissingTubeLength), G4RotationMatrix(), "HACoating");
}




/**
 * Placement of the PMT and definition of LogicalBorderSurfaces in case internal reflections are needed.
 * @param pPosition G4ThreeVector with position of the module (as in G4PVPlacement())
 * @param pRotation G4RotationMatrix with rotation of the module (as in G4PVPlacement())
 * @param pMother G4LogicalVolume where the module is going to be placed (as in G4PVPlacement())
 * @param pNameExtension G4String name of the physical volume. You should not have two physicals with the same name
 */
void OMSimPMTConstruction::placeIt(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4LogicalVolume*& pMother, G4String pNameExtension)
{
    abcDetectorComponent::placeIt(pPosition, pRotation, pMother, pNameExtension);
    
   new G4LogicalBorderSurface("PMT_mirrorglass", mVacuumBackPhysical, mLastPhysicals["PMT"], mData->getOpticalSurface("Refl_PMTSideMirror"));
   new G4LogicalBorderSurface("PMT_mirrorglass", mLastPhysicals["PMT"], mVacuumBackPhysical, mData->getOpticalSurface("Refl_PMTSideMirror"));
}

/**
 * @see PMT::placeIt
 * @param pTransform G4Transform3D with position & rotation of PMT
 */
void OMSimPMTConstruction::placeIt(G4Transform3D pTransform, G4LogicalVolume*& pMother, G4String pNameExtension)
{
    abcDetectorComponent::placeIt(pTransform, pMother, pNameExtension);

   new G4LogicalBorderSurface("PMT_mirrorglass", mVacuumBackPhysical, mLastPhysicals["PMT"], mData->getOpticalSurface("Refl_PMTSideMirror"));
   new G4LogicalBorderSurface("PMT_mirrorglass", mLastPhysicals["PMT"], mVacuumBackPhysical, mData->getOpticalSurface("Refl_PMTSideMirror"));
}


/**
 * The basic shape of the PMT is constructed twice, once for the external solid and once for the internal. A subtraction of these two shapes would yield the glass envelope of the PMT. The function calls either BulbConstructionSimple or BulbConstructionFull, depending on the data provided and simulation type. In case only the frontal curvate of the photocathode has to be well constructed, it calls BulbConstructionSimple. BulbConstructionFull constructs the neck of the PMT precisely, but it needs to have the fit data of the PMT type and is only needed if internal reflections are simulated.
 * @see BulbConstructionSimple
 * @see BulbConstructionFull
 */
std::tuple<G4VSolid*, G4VSolid*> OMSimPMTConstruction::GetBulbSolid(G4String pSide)
{
    G4SubtractionSolid* lVacuumPhotocathodeSolid;
    G4String lBulbBackShape = mData->getValue<G4String>(mSelectedPMT, "jBulbBackShape");
    return BulbConstructionFull(pSide);
    // if (lBulbBackShape == "Simple") mSimpleBulb = true;

    // if (mSimpleBulb || !mInternalReflections)
    // {
    //     return BulbConstructionSimple(pSide);
    // }
    // else
    // {
    //     return BulbConstructionFull(pSide);
    // }
}

/**
 * Construction of the basic shape of the PMT.
 * @return tuple of G4UnionSolid (the outer shape) and G4SubtractionSolid (the photocathode volume part)
 */
std::tuple<G4VSolid*, G4VSolid*> OMSimPMTConstruction::BulbConstructionSimple(G4String pSide)
{
    G4VSolid* lBulbSolid = FrontalBulbConstruction(pSide);
    // Defining volume with boundaries of photocathode volume
    G4Tubs* lLargeTube = new G4Tubs("LargeTube", 0, mEllipseXYaxis, 50 * cm, 0, 2 * CLHEP::pi);
    G4SubtractionSolid* lPhotocathodeSide = new G4SubtractionSolid("SubstractionPhotocathodeSide", lBulbSolid, lLargeTube, 0, G4ThreeVector(0, 0, -50 * cm));

    G4Tubs* lBulkSolid = new G4Tubs("Bulb bulk solid", 0.0, 0.5 * mTubeWidth, mMissingTubeLength, 0, 2 * CLHEP::pi);
    lBulbSolid = new G4UnionSolid("Bulb tube solid", lBulbSolid, lBulkSolid, 0, G4ThreeVector(0, 0, -mMissingTubeLength));
    return std::make_tuple(lBulbSolid, lPhotocathodeSide);
}

/**
 * Construction of the basic shape of the PMT for a full paramterised PMT. This is needed if internal reflections are simulated.
 * @return tuple of G4UnionSolid (the outer shape) and G4SubtractionSolid (the photocathode volume part)
 */
std::tuple<G4VSolid*, G4VSolid*> OMSimPMTConstruction::BulbConstructionFull(G4String pSide)
{
    G4double lLineFitSlope = mData->getValueWithUnit(mSelectedPMT, pSide + ".jLineFitSlope");
    G4double lEllipseConeTransition_x = mData->getValueWithUnit(mSelectedPMT, pSide + ".jEllipseConeTransition_x");
    G4double lEllipseConeTransition_y = mData->getValueWithUnit(mSelectedPMT, pSide + ".jEllipseConeTransition_y");
    G4double lConeTorusTransition_x = mData->getValueWithUnit(mSelectedPMT, pSide + ".jConeTorusTransition_x");
    G4double lTorusCircleR = mData->getValueWithUnit(mSelectedPMT, pSide + ".jTorusCircleR");
    G4double lTorusCirclePos_x = mData->getValueWithUnit(mSelectedPMT, pSide + ".jTorusCirclePos_x");
    G4double lTorusCirclePos_y = mData->getValueWithUnit(mSelectedPMT, pSide + ".jTorusCirclePos_y");
    G4double lTorusTubeTransition_y = mData->getValueWithUnit(mSelectedPMT, pSide + ".jTorusTubeTransition_y");

    G4VSolid* lBulbSolid = FrontalBulbConstruction(pSide);
    // Defining volume with boundaries of photocathode volume
    G4Tubs* lLargeTube = new G4Tubs("LargeTube", 0, mEllipseXYaxis, 50 * cm, 0, 2 * CLHEP::pi);
    G4SubtractionSolid* lPhotocathodeSide = new G4SubtractionSolid("SubstractionPhotocathodeSide", lBulbSolid, lLargeTube, 0, G4ThreeVector(0, 0, -50 * cm));

    // Rest of tube
    G4Tubs* lBulkSolid = new G4Tubs("Bulb bulk solid", 0.0, 0.5 * mTubeWidth, mMissingTubeLength, 0, 2 * CLHEP::pi);

    // Creating Cone
    G4double lConeLength_x = lEllipseConeTransition_x - lConeTorusTransition_x;
    G4double lConeHalfHeight = lLineFitSlope * lConeLength_x * 0.5;
    G4Cons* lCone = new G4Cons("Solid substraction cone", lConeTorusTransition_x, lConeTorusTransition_x + mTubeWidth, lEllipseConeTransition_x, lEllipseConeTransition_x + mTubeWidth, lConeHalfHeight, 0, 2 * CLHEP::pi);

    // Cone is substracted from frontal volume
    G4double lConeEllipse_y = lEllipseConeTransition_y - mEllipsePos_y - lConeHalfHeight;
    G4SubtractionSolid* lBulbSolidSubstractions = new G4SubtractionSolid("Substracted solid bulb", lBulbSolid,
        lCone, 0, G4ThreeVector(0, 0, lConeEllipse_y));

    // Creating Torus
    G4Torus* lTorus = new G4Torus("Solid substraction torus", 0.0, lTorusCircleR, lTorusCirclePos_x, 0, 2 * CLHEP::pi);
    G4double lTorusToEllipse = lTorusCirclePos_y - mEllipsePos_y;
    G4Tubs* lTubeEdge = new G4Tubs("Solid edge of torus", lTorusCirclePos_x, lEllipseConeTransition_x + mTubeWidth, lTorusCirclePos_x * 0.5, 0, 2 * CLHEP::pi);

    G4UnionSolid* lTorusTubeEdge = new G4UnionSolid("Solid torus with cylindrical edges", lTorus, lTubeEdge, 0, G4ThreeVector(0, 0, 0));

    // Create Tube for substracting cone and torus
    G4double lSubstractionTubeLength = lEllipseConeTransition_y - lTorusTubeTransition_y;
    G4Tubs* lSubstractionTube = new G4Tubs("substracion_tube", 0.0, lEllipseConeTransition_x, 0.5 * lSubstractionTubeLength, 0, 2 * CLHEP::pi);

    G4double lSTubeEllipse_y = lEllipseConeTransition_y - mEllipsePos_y - lSubstractionTubeLength * 0.5;

    G4SubtractionSolid* lBulbBack = new G4SubtractionSolid("Solid back of PMT", lSubstractionTube, lCone, 0, G4ThreeVector(0, 0, lConeEllipse_y - lSTubeEllipse_y));
    lBulbBack = new G4SubtractionSolid("Solid back of PMT", lBulbBack, lTorusTubeEdge, 0, G4ThreeVector(0, 0, lTorusToEllipse - lSTubeEllipse_y));

    lBulbSolid = new G4UnionSolid("Bulb tube solid", lBulbSolid, lBulkSolid, 0, G4ThreeVector(0, 0, -mMissingTubeLength));

    return std::make_tuple(lBulbSolid, lPhotocathodeSide);
}

/**
 * Creates and positions a thin disk behind the photocathode volume in order to shield photons coming from behind the PMT. Only used when internal reflections are turned off.
 */
void OMSimPMTConstruction::CathodeBackShield(G4LogicalVolume* pPMTinner)
{
    ReadGlobalParameters("jInnerShape");
    G4double lShieldWidth = 0.5 * mm;
    G4double lShieldZPos = 0.1 * mm;
    G4double lFurthestZ = lShieldWidth + lShieldZPos;
    G4double lShieldRad = mEllipseXYaxis * std::sqrt(1 - std::pow(lFurthestZ, 2.) / std::pow(mEllipseZaxis, 2.));
    G4Tubs* lShieldSolid = new G4Tubs("Shield solid", 0, lShieldRad - 0.05 * mm, lShieldWidth / 2, 0, 2 * CLHEP::pi);
    G4LogicalVolume* lShieldLogical = new G4LogicalVolume(lShieldSolid, mData->getMaterial("NoOptic_Absorber"), "Shield logical");
    new G4PVPlacement(0, G4ThreeVector(0, 0, -lShieldZPos), lShieldLogical, "Shield physical", pPMTinner, false, 0, mCheckOverlaps);
    lShieldLogical->SetVisAttributes(mBlackVis);
}

/**
 * Construction & placement of the dynode system entrance for internal reflections. Currently only geometry for Hamamatsu R15458.
 * @param pMother LogicalVolume of the mother, where the dynode system entrance is placed (vacuum volume)
 */
void OMSimPMTConstruction::DynodeSystemConstructionCAD(G4LogicalVolume* pMother)
{

    auto lSupportStructureMesh = CADMesh::TessellatedMesh::FromOBJ("../data/CADmeshes/PMT/streifen.obj");
    auto lFrontalPlateMesh = CADMesh::TessellatedMesh::FromOBJ("../data/CADmeshes/PMT/frontalPlateonly.obj");
    auto lDynodesMesh = CADMesh::TessellatedMesh::FromOBJ("../data/CADmeshes/PMT/dynodes.obj");

    G4double lDynodeOffset = mData->getValueWithUnit(mSelectedPMT, "jDynodeCADOffset");
    G4double lScale = mData->getValueWithUnit(mSelectedPMT, "jDynodeCADscale");

    G4ThreeVector lCADoffset = G4ThreeVector(0, 0, -(lDynodeOffset-GetDistancePMTCenterToPMTtip()));// -(54.2 + 19.5));
    lSupportStructureMesh->SetOffset(lCADoffset);
    lFrontalPlateMesh->SetOffset(lCADoffset);
    lDynodesMesh->SetOffset(lCADoffset);
    lSupportStructureMesh->SetScale(lScale);
    lFrontalPlateMesh->SetScale(lScale);
    lDynodesMesh->SetScale(lScale);

    G4RotationMatrix* lRot = new G4RotationMatrix();
    lRot->rotateZ(90 * deg);

    G4LogicalVolume* lSupportStructure = new G4LogicalVolume(lSupportStructureMesh->GetSolid(), mData->getMaterial("NoOptic_Absorber"), "logical", 0, 0, 0);
    G4LogicalVolume* lFrontalPlate = new G4LogicalVolume(lFrontalPlateMesh->GetSolid(), mData->getMaterial("NoOptic_Absorber"), "logical", 0, 0, 0); 
    G4LogicalVolume* lDynodes = new G4LogicalVolume(lDynodesMesh->GetSolid(), mData->getMaterial("NoOptic_Absorber"), "logical", 0, 0, 0); 
    
    new G4LogicalSkinSurface("SkinFrontalPlate", lFrontalPlate, mData->getOpticalSurface("Refl_PMTFrontPlate"));
    new G4LogicalSkinSurface("SkinSupportStructure", lSupportStructure, mData->getOpticalSurface("Refl_AluminiumGround"));
    new G4LogicalSkinSurface("SkinDynodes", lDynodes, mData->getOpticalSurface("Refl_Dynode"));

    //lAbsorbers->SetVisAttributes(mAbsorberVis);
    //new G4PVPlacement( lRot , G4ThreeVector(0, 0, 0) , lAbsorbers, "DynodeSystemAbsorbers" , pMother, false, 0, mCheckOverlaps);
    lFrontalPlate->SetVisAttributes(mBlueVis);
    new G4PVPlacement(lRot, G4ThreeVector(0, 0, 0), lFrontalPlate, "frontalPlate", pMother, false, 0, mCheckOverlaps);

    lSupportStructure->SetVisAttributes(mBoardVis);
    new G4PVPlacement(lRot, G4ThreeVector(0, 0, 0), lSupportStructure, "DynodeSupportStructure", pMother, false, 0, mCheckOverlaps);

    lDynodes->SetVisAttributes(mRedVis);
    new G4PVPlacement(lRot, G4ThreeVector(0, 0, 0), lDynodes, "Dynodes", pMother, false, 0, mCheckOverlaps);
    
    ReadGlobalParameters("jInnerShape");
    G4Tubs* lShieldSolid = new G4Tubs("Shield solid", 0, 0.5*mTubeWidth - 0.05 * mm, 0.05 * mm / 2, 0, 2 * CLHEP::pi);
    G4LogicalVolume* lShieldLogical = new G4LogicalVolume(lShieldSolid, mData->getMaterial("NoOptic_Absorber"), "logical", 0, 0, 0); 
    lShieldLogical->SetVisAttributes(mAbsorberVis);
    new G4PVPlacement(lRot, G4ThreeVector(0, 0, -0.6*2*mMissingTubeLength), lShieldLogical, "BackShield", pMother, false, 0, mCheckOverlaps);
}


/**
 * Reads the parameter table and assigns the value and dimension of member variables.
 */
void OMSimPMTConstruction::ReadGlobalParameters(G4String pSide)
{
    mOutRad = mData->getValueWithUnit(mSelectedPMT, pSide + ".jOutRad");
    mEllipseXYaxis = mData->getValueWithUnit(mSelectedPMT, pSide + ".jEllipseXYaxis");
    mEllipseZaxis = mData->getValueWithUnit(mSelectedPMT, pSide + ".jEllipseZaxis");
    mSpherePos_y = mData->getValueWithUnit(mSelectedPMT, pSide + ".jSpherePos_y");
    mEllipsePos_y = mData->getValueWithUnit(mSelectedPMT, pSide + ".jEllipsePos_y");
    mSphereEllipseTransition_r = mData->getValueWithUnit(mSelectedPMT, pSide + ".jSphereEllipseTransition_r");
    if (pSide != "jPhotocathodeInnerSide") {
        mTotalLenght = mData->getValueWithUnit(mSelectedPMT, pSide + ".jTotalLenght");
        mTubeWidth = mData->getValueWithUnit(mSelectedPMT, pSide + ".jTubeWidth");
        G4double lFrontToEllipse_y = mOutRad + mSpherePos_y - mEllipsePos_y;
        mMissingTubeLength = (mTotalLenght - lFrontToEllipse_y) * 0.5 * mm;
    }
}


G4VSolid* OMSimPMTConstruction::FrontalBulbConstruction(G4String pSide)
{
    G4String lFrontalShape = mData->getValue<G4String>(mSelectedPMT, "jFrontalShape");
    ReadGlobalParameters(pSide);
    if (lFrontalShape == "SphereEllipse")
        return SphereEllipsePhotocathode();
    else if (lFrontalShape == "Sphere2Ellipses")
        return SphereDoubleEllipsePhotocathode(pSide);
    else if (lFrontalShape == "TwoEllipses")
        return DoubleEllipsePhotocathode(pSide);
    else if (lFrontalShape == "SingleEllipse")
        return EllipsePhotocathode();
}


/**
 * Construction of the photocathode layer.
 * @return G4SubtractionSolid
 */
G4SubtractionSolid* OMSimPMTConstruction::PhotocathodeLayerConstruction()
{
    checkPhotocathodeThickness();

    G4VSolid* lInnerBoundarySolid = FrontalBulbConstruction("jPhotocathodeInnerSide");
    G4VSolid* lOutBoundarySolid = FrontalBulbConstruction("jInnerShape");

    G4SubtractionSolid* lShellSolid = new G4SubtractionSolid("ShellOfFrontalBulb", lOutBoundarySolid, lInnerBoundarySolid, 0, G4ThreeVector(0, 0, 0));

    G4Ellipsoid* lBorderCut = new G4Ellipsoid("Solid Bulb Ellipsoid", mEllipseXYaxis, mEllipseXYaxis, mEllipseZaxis-220*nm);

    G4Tubs* lLargeTube = new G4Tubs("LargeTube", 0, 2 * mEllipseXYaxis, 50 * cm, 0, 2 * CLHEP::pi);

    G4SubtractionSolid* lNoBorderSolid = new G4SubtractionSolid("SubstractionPhotocathodeSide", lShellSolid, lBorderCut, 0, G4ThreeVector(0,0,0));
    //appendComponent(lHACoatingCut, lHACoatingLogical, G4ThreeVector(0, 0, -mMissingTubeLength), G4RotationMatrix(), "HACoating"); 

    return new G4SubtractionSolid("SubstractionPhotocathodeSide", lNoBorderSolid, lLargeTube, 0, G4ThreeVector(0, 0, -50 * cm));
    //return new G4SubtractionSolid("SubstractionPhotocathodeSide", lOutBoundarySolid, lLargeTube, 0, G4ThreeVector(0, 0, -50 * cm));
}


void OMSimPMTConstruction::checkPhotocathodeThickness(){
    G4String lSide = "jPhotocathodeInnerSide";
    G4double lOutRad = mData->getValueWithUnit(mSelectedPMT, lSide + ".jOutRad");
    G4double lEllipseXYaxis = mData->getValueWithUnit(mSelectedPMT, lSide + ".jEllipseXYaxis");
    G4double lEllipseZaxis = mData->getValueWithUnit(mSelectedPMT, lSide + ".jEllipseZaxis");
    G4double lSpherePos_y = mData->getValueWithUnit(mSelectedPMT, lSide + ".jSpherePos_y");
    G4double lEllipsePos_y = mData->getValueWithUnit(mSelectedPMT, lSide + ".jEllipsePos_y");

    lSide = "jInnerShape";
    G4cout << "lOutRad " <<  (mData->getValueWithUnit(mSelectedPMT, lSide + ".jOutRad")-lOutRad)/nm << G4endl;
    G4cout << "lEllipseXYaxis " << (mData->getValueWithUnit(mSelectedPMT, lSide + ".jEllipseXYaxis")-lEllipseXYaxis)/nm << G4endl;
    G4cout << "lEllispesZaxis " << (mData->getValueWithUnit(mSelectedPMT, lSide + ".jEllipseZaxis")-lEllipseZaxis)/nm << G4endl;
    G4cout << "lSpherePos_y " << (mData->getValueWithUnit(mSelectedPMT, lSide + ".jSpherePos_y")-lSpherePos_y)/nm << G4endl;
    G4cout << "lEllipsePos_y " << (mData->getValueWithUnit(mSelectedPMT, lSide + ".jEllipsePos_y")-lEllipsePos_y)/nm << G4endl;

}

/**
 * Construction of the frontal part of the PMT following the fits of the technical drawings. PMTs constructed with SphereEllipsePhotocathode were fitted with a sphere and an ellipse.
 * @return G4UnionSolid lBulbSolid the frontal solid of the PMT
 */
G4UnionSolid* OMSimPMTConstruction::SphereEllipsePhotocathode()
{
    G4double lSphereAngle = asin(mSphereEllipseTransition_r / mOutRad);
    // PMT frontal glass envelope as union of sphere and ellipse
    G4Ellipsoid* lBulbEllipsoid = new G4Ellipsoid("Solid Bulb Ellipsoid", mEllipseXYaxis, mEllipseXYaxis, mEllipseZaxis);
    G4Sphere* lBulbSphere = new G4Sphere("Solid Bulb Ellipsoid", 0.0, mOutRad, 0, 2 * CLHEP::pi, 0, lSphereAngle);
    G4UnionSolid* lBulbSolid = new G4UnionSolid("Solid Bulb", lBulbEllipsoid, lBulbSphere, 0, G4ThreeVector(0, 0, mSpherePos_y - mEllipsePos_y));
    return lBulbSolid;
}

G4UnionSolid* OMSimPMTConstruction::EllipsePhotocathode()
{
    G4double lSphereAngle = asin(mSphereEllipseTransition_r / mOutRad);
    // PMT frontal glass envelope as union of sphere and ellipse
    G4Ellipsoid* lBulbEllipsoid = new G4Ellipsoid("Solid Bulb Ellipsoid", mEllipseXYaxis, mEllipseXYaxis, mEllipseZaxis);
    G4Sphere* lBulbSphere = new G4Sphere("Solid Bulb Ellipsoid", 0.0, 0.1, 0, 2 * CLHEP::pi, 0, lSphereAngle);
    G4UnionSolid* lBulbSolid = new G4UnionSolid("Solid Bulb", lBulbEllipsoid, lBulbSphere, 0, G4ThreeVector(0, 0, mSpherePos_y - mEllipsePos_y));
    return lBulbSolid;
}

/**
 * Construction of the frontal part of the PMT following the fits of the technical drawings. PMTs constructed with SphereDoubleEllipsePhotocathode were fitted with a sphere and two ellipses.
 * @return G4UnionSolid lBulbSolid the frontal solid of the PMT
 */
G4UnionSolid* OMSimPMTConstruction::SphereDoubleEllipsePhotocathode(G4String pSide)
{
    G4double lEllipseXYaxis_2 = mData->getValueWithUnit(mSelectedPMT, pSide + ".jEllipseXYaxis_2");
    G4double lEllipseZaxis_2 = mData->getValueWithUnit(mSelectedPMT, pSide + ".jEllipseZaxis_2");
    G4double lEllipsePos_y_2 = mData->getValueWithUnit(mSelectedPMT, pSide + ".jEllipsePos_y_2");

    G4double lSphereAngle = asin(mSphereEllipseTransition_r / mOutRad);
    // PMT frontal glass envelope as union of sphere and ellipse
    G4Ellipsoid* lBulbEllipsoid = new G4Ellipsoid("Solid Bulb Ellipsoid", mEllipseXYaxis, mEllipseXYaxis, mEllipseZaxis);
    G4Sphere* lBulbSphere = new G4Sphere("Solid Bulb Ellipsoid", 0.0, mOutRad, 0, 2 * CLHEP::pi, 0, lSphereAngle);
    G4UnionSolid* lBulbSolid = new G4UnionSolid("Solid Bulb", lBulbEllipsoid, lBulbSphere, 0, G4ThreeVector(0, 0, mSpherePos_y - mEllipsePos_y));
    G4Ellipsoid* lBulbEllipsoid_2 = new G4Ellipsoid("Solid Bulb Ellipsoid 2", lEllipseXYaxis_2, lEllipseXYaxis_2, lEllipseZaxis_2);
    G4double lExcess = mEllipsePos_y - lEllipsePos_y_2;
    G4Tubs* lSubtractionTube = new G4Tubs("substracion_tube_large_ellipsoid", 0.0, lEllipseXYaxis_2 * 2, 0.5 * mTotalLenght, 0, 2 * CLHEP::pi);
    G4SubtractionSolid* lSubstractedLargeEllipsoid = new G4SubtractionSolid("Substracted Bulb Ellipsoid 2", lBulbEllipsoid_2, lSubtractionTube, 0, G4ThreeVector(0, 0, lExcess - mTotalLenght * 0.5));
    lBulbSolid = new G4UnionSolid("Solid Bulb", lBulbSolid, lSubstractedLargeEllipsoid, 0, G4ThreeVector(0, 0, lEllipsePos_y_2 - mEllipsePos_y));
    return lBulbSolid;
}


/**
 * Construction of the frontal part of the PMT following the fits of the technical drawings. PMTs constructed with DoubleEllipsePhotocathode were fitted with two ellipses.
 * @return G4UnionSolid lBulbSolid the frontal solid of the PMT
 */
G4UnionSolid* OMSimPMTConstruction::DoubleEllipsePhotocathode(G4String pSide)
{
    G4double lEllipseXYaxis_2 = mData->getValueWithUnit(mSelectedPMT, pSide + ".jEllipseXYaxis_2");
    G4double lEllipseZaxis_2 = mData->getValueWithUnit(mSelectedPMT, pSide + ".jEllipseZaxis_2");
    G4double lEllipsePos_y_2 = mData->getValueWithUnit(mSelectedPMT, pSide + ".jEllipsePos_y_2");

    G4double lSphereAngle = asin(mSphereEllipseTransition_r / mOutRad);
    // PMT frontal glass envelope as union of sphere and ellipse
    G4Ellipsoid* lBulbEllipsoid = new G4Ellipsoid("Solid Bulb Ellipsoid", mEllipseXYaxis, mEllipseXYaxis, mEllipseZaxis);
    G4Ellipsoid* lBulbEllipsoid_2 = new G4Ellipsoid("Solid Bulb Ellipsoid 2", lEllipseXYaxis_2, lEllipseXYaxis_2, lEllipseZaxis_2);
    G4double lExcess = mEllipsePos_y - lEllipsePos_y_2;
    G4Tubs* lSubtractionTube = new G4Tubs("substracion_tube_large_ellipsoid", 0.0, lEllipseXYaxis_2 * 2, 0.5 * mTotalLenght, 0, 2 * CLHEP::pi);
    G4SubtractionSolid* lSubstractedLargeEllipsoid = new G4SubtractionSolid("Substracted Bulb Ellipsoid 2", lBulbEllipsoid_2, lSubtractionTube, 0, G4ThreeVector(0, 0, lExcess - mTotalLenght * 0.5));
    G4UnionSolid* lBulbSolid = new G4UnionSolid("Solid Bulb", lBulbEllipsoid, lSubstractedLargeEllipsoid, 0, G4ThreeVector(0, 0, lEllipsePos_y_2 - mEllipsePos_y));
    return lBulbSolid;
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                Main class methods
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

 /**
  * Returns the distance between the 0.0 position of the PMT solid volume and the plane normal to the PMT frontal tip.
  * @return G4double
  */
G4double OMSimPMTConstruction::GetDistancePMTCenterToPMTtip()
{
    ReadGlobalParameters("jOuterShape");
    return mOutRad + mSpherePos_y - mEllipsePos_y;
}
/**
 * Returns the maximal radius of the frontal part of the PMT.
 * @return G4double
 */
G4double OMSimPMTConstruction::GetMaxPMTMaxRadius()
{
    ReadGlobalParameters("jOuterShape");
    return mEllipseXYaxis;
}
/**
 * Returns the solid of the constructed PMT.
 * @return G4UnionSolid of the PMT
 */
G4VSolid* OMSimPMTConstruction::GetPMTSolid()
{
    return mComponents.at("PMT").VSolid;
}

/**
 * It returns the member logical volume. To be used if you want to append the PMT in an abcDetectorComponent. For placements use placeIt method.
 */
G4LogicalVolume* OMSimPMTConstruction::GetLogicalVolume()
{
    return mComponents.at("PMT").VLogical;
}

/**
 * Select PMT model to use and assigns mPMT class.
 * @param G4String pPMTtoSelect string with the name of the PMT model
 */
void OMSimPMTConstruction::SelectPMT(G4String pPMTtoSelect)
{
    if (pPMTtoSelect.substr(0, 6) == "argPMT")
    {
        const G4String lPMTTypes[] = { "pmt_Hamamatsu_R15458_20nm", "pmt_Hamamatsu_R7081", "pmt_Hamamatsu_4inch", "pmt_Hamamatsu_R5912_20_100" };
        pPMTtoSelect = lPMTTypes[OMSimCommandArgsTable::getInstance().get<G4int>("pmt_model")];
    }
    mSelectedPMT = pPMTtoSelect;

    //Check if requested PMT is in the table of PMTs
    if (mData->checkIfKeyInTable(pPMTtoSelect))
    {//if found
        G4String mssg = pPMTtoSelect + " selected.";
        log_notice(mssg);
    }
    else
    {
        log_critical("Selected PMT not in PMT tree, please check that requested PMT exists in data folder.");
    }
}

/**
 * The PMT is constructed allowing internal reflections (in the mirrored part opposite to the photocathode). Do not use it if you really don't need it!!
 */
void OMSimPMTConstruction::SimulateInternalReflections()
{
    mInternalReflections = true;
    if (mSelectedPMT.substr(0,20) == "pmt_Hamamatsu_R15458")
        mDynodeSystem = true;
    else
    {
        log_warning("Internal reflections inside PMT is on, but this PMT has no Dynode system defined, only vacuum! I will continue with internal reflections anyway...");
        mDynodeSystem = false;
    }
    if (mConstructionFinished) {
        log_warning("You should call this function before Construction(), otherwise we have to construct everything twice...");
        construction();
    }
}


void OMSimPMTConstruction::SimulateHACoating()
{
    mHACoatingBool = true;
    if (mConstructionFinished) {
        log_warning("You should call this function before Construction(), otherwise we have to construct everything twice...");
        construction();
    }
}
