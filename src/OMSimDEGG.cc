/** @file OMSimDEGG.cc
 *  @brief Construction of DEGG.
 *
 *  @author Berit Schlüter, modified by Martin Unland
 *  @date July 2022
 *
 *  @version Geant4 10.7
 *
 */


#include "OMSimDEGG.hh"
#include "OMSimDEGGHarness.hh" 
#include "abcDetectorComponent.hh"
#include <dirent.h>
#include <stdexcept>
#include <cstdlib>

#include "G4Cons.hh"
#include "G4Ellipsoid.hh"
#include "G4EllipticalTube.hh"
#include "G4Sphere.hh"
#include "G4IntersectionSolid.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4Orb.hh"
#include "G4Polycone.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4Transform3D.hh"
#include "G4Tubs.hh"
#include "G4UnionSolid.hh"
#include <G4UnitsTable.hh>
#include "G4VisAttributes.hh"
#include "G4Torus.hh"
#include "CADMesh.hh" 




DEgg::DEgg(OMSimInputData* pData, G4bool pPlaceHarness) {
   mData = pData;
   mPMTManager = new OMSimPMTConstruction(mData);
   mPMTManager->SelectPMT("pmt_Hamamatsu_R5912_20_100");
   mPMTManager->Construction();
   Construction(); // always before harness, otherwise harness will be deleted :(
/*
   if (pPlaceHarness) {
      mHarness = new dEGGHarness(this, mData);
      IntegrateDetectorComponent(mHarness, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "");
   }*/
}

/**
 * @brief Construction of the whole DEGG. If you want to change any component, you have to change it at the specific function.
 *
 */
void DEgg::Construction() 
{
   mComponents.clear();
   //Create pressure vessel and inner volume
   G4VSolid* lOuterGlass = CreateEggSolid(mData->GetValueWithUnit(mDataKey, "jOutSegments1"),
      mData->GetValueWithUnit(mDataKey, "jOutSphereRadiusMax"),
      mData->GetValueWithUnit(mDataKey, "jOutSphereDtheta"),
      mData->GetValueWithUnit(mDataKey, "jOutTransformZ"),
      mData->GetValueWithUnit(mDataKey, "jOutTorusRadius1"),
      mData->GetValueWithUnit(mDataKey, "jOutCenterOfTorusRadius1"),
      mData->GetValueWithUnit(mDataKey, "jOutSegments2"),
      mData->GetValueWithUnit(mDataKey, "jOutTorusRadius2"),
      mData->GetValueWithUnit(mDataKey, "jOutCenterOfTorusRadius2"),
      mData->GetValueWithUnit(mDataKey, "jOutCenterOfTorusZ2"),
      mData->GetValueWithUnit(mDataKey, "jOutTorusZmin2"),
      mData->GetValueWithUnit(mDataKey, "jOutTorusZmax2"),
      mData->GetValueWithUnit(mDataKey, "jOutTorusZ0"),
      mData->GetValueWithUnit(mDataKey, "jOutTorusTransformZ"));

   G4VSolid* lInternalVolume = CreateEggSolid(mData->GetValueWithUnit(mDataKey, "jInnSegments1"),
      mData->GetValueWithUnit(mDataKey, "jInnSphereRadiusMax"),
      mData->GetValueWithUnit(mDataKey, "jInnSphereDtheta"),
      mData->GetValueWithUnit(mDataKey, "jInnTransformZ"),
      mData->GetValueWithUnit(mDataKey, "jInnTorusRadius1"),
      mData->GetValueWithUnit(mDataKey, "jInnCenterOfTorusRadius1"),
      mData->GetValueWithUnit(mDataKey, "jInnSegments2"),
      mData->GetValueWithUnit(mDataKey, "jInnTorusRadius2"),
      mData->GetValueWithUnit(mDataKey, "jInnCenterOfTorusRadius2"),
      mData->GetValueWithUnit(mDataKey, "jInnCenterOfTorusZ2"),
      mData->GetValueWithUnit(mDataKey, "jInnTorusZmin2"),
      mData->GetValueWithUnit(mDataKey, "jInnTorusZmax2"),
      mData->GetValueWithUnit(mDataKey, "jInnTorusZ0"),
      mData->GetValueWithUnit(mDataKey, "jInnTorusTransformZ"));

   
   // Make box to substract empty space
   G4double lGelHeight = mData->GetValueWithUnit(mDataKey, "jGelHeight");
   G4Box* lSubstractionBox = new G4Box("SubstractionBox", 20*cm, 20*cm, lGelHeight);
   G4LogicalVolume* lLogicalDummy = new G4LogicalVolume(lSubstractionBox, mData->GetMaterial("Ri_Air"), "Temp");

   //Append all internal components
   AppendComponent(lSubstractionBox, lLogicalDummy, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "SubstractionBox");
   AppendPMTs(); 
   

   //Substract all internal components to internal volume to obtain gel and append it
   G4VSolid* lGelLayers = SubstractToVolume(lInternalVolume, G4ThreeVector(0, 0, 0),  G4RotationMatrix(), "DeggGelLayersSolid");
   G4LogicalVolume* lGelLogical = new G4LogicalVolume(lGelLayers, mData->GetMaterial("RiAbs_Gel_Shin-Etsu"), "DeggGelLayersLogical");
   AppendComponent(lGelLayers, lGelLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "DeggGelLayers");

   // Delete dummy box from internal components
   DeleteComponent("SubstractionBox");
   //AppendInternalComponentsFromCAD();

   //Logicals
   G4LogicalVolume* lDEggGlassLogical = new G4LogicalVolume(lOuterGlass, mData->GetMaterial("RiAbs_Glass_Okamoto_DOUMEKI"), "Glass_phys");
   G4LogicalVolume* lInnerVolumeLogical = new G4LogicalVolume(lInternalVolume, mData->GetMaterial("Ri_Air"), "InnerVolume");

   //Placements
   //place all internal components in internal volume 
   PlaceIt(G4ThreeVector(0,0,0),  G4RotationMatrix(), lInnerVolumeLogical, "");

   //place internal volume in glass
   new G4PVPlacement(new G4RotationMatrix(), G4ThreeVector(0, 0, 0), lInnerVolumeLogical, "VacuumGlass", lDEggGlassLogical, false, 0, mCheckOverlaps);   

   //Delete all internal components from dictionary, as they were placed in a volume inside the largest volume.
   mComponents.clear(); 

   //Add glass volume to component map
   //AppendComponent(lInternalVolume, lInnerVolumeLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "Internal");
   AppendComponent(lOuterGlass, lDEggGlassLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PressureVessel");
   //AppendPressureVesselFromCAD(); 
   
   // ---------------- visualisation attributes --------------------------------------------------------------------------------
   lDEggGlassLogical->SetVisAttributes(mGlassVis);
   lInnerVolumeLogical->SetVisAttributes(G4VisAttributes::GetInvisible());
}


/**
 * @brief Construction the PMT of the DEGG. PMTs are placed in the logical of the Gel (see PlaceGel()).
 *
 */
void DEgg::AppendPMTs() {
   G4double lPmtDistance = mData->GetValueWithUnit(mDataKey, "jPmtDistance");
   G4RotationMatrix lRot = G4RotationMatrix();
   lRot.rotateY(180 * deg);

   AppendComponent(mPMTManager->GetPMTSolid(),
                   mPMTManager->GetLogicalVolume(),
                   G4ThreeVector(0, 0, lPmtDistance), 
                   G4RotationMatrix(), 
                   "PMT_1");

   AppendComponent(mPMTManager->GetPMTSolid(),
                   mPMTManager->GetLogicalVolume(),
                   G4ThreeVector(0, 0, -lPmtDistance), 
                   lRot, 
                   "PMT_2");
}
/**
 * @brief Placement of the SupportStructure (from CAD)
 *
 */
void DEgg::AppendInternalComponentsFromCAD()
{
   G4String lFilePath = mData->GetValue<G4String>(mDataKey, "jInternalCADFile");
   G4double lCADScale = mData->GetValueWithUnit(mDataKey, "jInternalCADScale");
   G4cout << "using the following CAD file for support structure: " << lFilePath << G4endl;

   //load mesh
   auto lMesh = CADMesh::TessellatedMesh::FromOBJ(lFilePath);

   G4ThreeVector lCADoffset = G4ThreeVector(mData->GetValueWithUnit(mDataKey, "jInternalCAD_x"), 
                                            mData->GetValueWithUnit(mDataKey, "jInternalCAD_y"), 
                                            mData->GetValueWithUnit(mDataKey, "jInternalCAD_z")); //measured from CAD file since origin =!= Module origin
   lMesh->SetScale(lCADScale);
   lMesh->SetOffset(lCADoffset*lCADScale);
   
   // Place all of the meshes it can find in the file as solids individually.
   for (auto iSolid : lMesh->GetSolids())
   {
      G4LogicalVolume* lSupportStructureLogical = new G4LogicalVolume(iSolid, mData->GetMaterial("NoOptic_Absorber"), "SupportStructureCAD_Logical");
      lSupportStructureLogical->SetVisAttributes(mAluVis);
      AppendComponent(iSolid, lSupportStructureLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "SupportStructureCAD");
   }
}


/**
 * @brief Placement of the SupportStructure (from CAD)
 *
 */
void DEgg::AppendPressureVesselFromCAD()
{
   G4String lFilePath = mData->GetValue<G4String>(mDataKey, "jPVCADFile");
   G4double lCADScale = mData->GetValueWithUnit(mDataKey, "jInternalCADScale");
   G4cout << "using the following CAD file for pressure vessel: " << lFilePath << G4endl;

   //load mesh
   auto lMesh = CADMesh::TessellatedMesh::FromOBJ(lFilePath);

   G4ThreeVector lCADoffset = G4ThreeVector(0,0,0); //measured from CAD file since origin =!= Module origin
   lMesh->SetScale(lCADScale);
   lMesh->SetOffset(lCADoffset*lCADScale);
   
   G4RotationMatrix* lRot = new G4RotationMatrix();
   lRot->rotateX(180*deg);
   
   
   // Place all of the meshes it can find in the file as solids individually.
   G4UnionSolid* lPressureVessel = new G4UnionSolid("CADPV", lMesh->GetSolids().at(0), lMesh->GetSolids().at(0), lRot, G4ThreeVector(0, -2*111*mm, 0));

    G4LogicalVolume* lSupportStructureLogical = new G4LogicalVolume(lPressureVessel, mData->GetMaterial("RiAbs_Glass_Okamoto_DOUMEKI"), "PressureVessel");
    lSupportStructureLogical->SetVisAttributes(mAluVis);

   G4RotationMatrix lRotNP = G4RotationMatrix();
   lRotNP.rotateX(90*deg);
   lSupportStructureLogical->SetVisAttributes(mGlassVis);
   
   AppendComponent(lPressureVessel, lSupportStructureLogical, G4ThreeVector(0, 0, 111*mm), lRotNP, "PressureVessel");
   
}

/**
 * Create D-Egg pressure vessel shape. From DOUMIKI, authors Chiba Collaborators. https://github.com/icecube/doumeki
 * @param pSegments_1 G4int
 * @param pSphereRmax G4double outer radius sphere
 * @param pSpheredTheta G4double delta theta angle of the segment
 * @param pSphereTransformZ G4double shift of sphere in z direction
 * @param pTorus1R G4double radius of small spindle torus sphere
 * @param pCenterOfTorus1R G4double distance from center of torus 1 to z-axis
 * @param pSegments_2 G4int
 * @param pTorus2R G4double radius of large spindle torus sphere
 * @param pCenterOfTorus2R G4double distance from center of pTorus2R to z-axis (signed)
 * @param pCenterOfTorus2_z G4double distance from center of pTorus2R to z-axis (signed)
 * @param pTorus2_Zmin G4double minimum z shift from z=0 in positive z direction
 * @param pTorus2_Zmax G4double maximum z shift from z=0 in positive z direction
 * @param pTorus2_Z0 G4double
 * @param pTorus1TransformZ G4double
 * @return return the outer or inner shape of the glass vessel
 *
 */
G4VSolid* DEgg::CreateEggSolid(G4int pSegments_1,
   G4double pSphereRmax,
   G4double pSpheredTheta,
   G4double pSphereTransformZ,
   G4double pTorus1R,
   G4double pCenterOfTorus1R,
   G4int pSegments_2,
   G4double pTorus2R,
   G4double pCenterOfTorus2R,
   G4double pCenterOfTorus2_z,
   G4double pTorus2_Zmin,
   G4double pTorus2_Zmax,
   G4double pTorus2_Z0,
   G4double pTorus1TransformZ)
{

   // Create Egg sphere 
   G4Sphere* lSphereSolid = new G4Sphere("sphere", 0, pSphereRmax, 0. * degree, 2 * M_PI, 0. * degree, pSpheredTheta);
   G4ThreeVector lCenterOfSphereUp(0, 0, pSphereTransformZ);

   //Torus Part 1
   //buidling small polycones, define full size of torus

   G4double lRInner[pSegments_1 + 1], lROuter[pSegments_1 + 1], lZPlane[pSegments_1 + 1];
   G4double lStep = pTorus1R / pSegments_1;
   std::vector<G4double> lTempZ, lTempOuter;

   G4double lR;
   for (G4int j = 0; j <= pSegments_1; ++j) {
      lR = sqrt((2 * pTorus1R - j * lStep) * j * lStep);
      lZPlane[j] = pTorus1R - j * lStep;
      lRInner[j] = 0.;
      lROuter[j] = pCenterOfTorus1R + lR;
   }

   G4Polycone* lTorusSolid1 = new G4Polycone("torus1", 0, 2 * M_PI, pSegments_1 + 1, lZPlane, lRInner, lROuter);

   //Torus Part 2
   //Building large sphere revolution
   G4double lRInner2[pSegments_2 + 1], lROuter2[pSegments_2 + 1], lZPlane2[pSegments_2 + 1];

   G4double lZMinRelative = pTorus2_Zmin - pCenterOfTorus2_z; //minimum z shift from center of torus in positive z direction
   G4double lZMaxRelative = pTorus2_Zmax - pCenterOfTorus2_z; //maximum z shift from center of torus in positive z direction
   lStep = (lZMaxRelative - lZMinRelative) / (pSegments_2 - 1);

   G4double lRelativeZMax2 = pTorus2_Zmax - pCenterOfTorus2_z;
   for (G4int j = 0; j <= pSegments_2 - 1; ++j) {
      lRInner2[j] = 0;
      lR = sqrt((pTorus2R + lRelativeZMax2 - j * lStep) * (pTorus2R - lRelativeZMax2 + j * lStep));
      lZPlane2[j] = pTorus2_Zmax - j * lStep;
      lROuter2[j] = pCenterOfTorus2R + lR;
   }
   lRInner2[pSegments_2] = 0;
   lZPlane2[pSegments_2] = 0.;
   lROuter2[pSegments_2] = pTorus2_Z0;

   G4Polycone* lTorusSolid2 = new G4Polycone("polycone2", 0, 2 * M_PI, pSegments_2 + 1, lZPlane2, lRInner2, lROuter2);

   //Create Vessel

   G4ThreeVector lCenterOfPolycone(0, 0, pTorus1TransformZ);

   G4UnionSolid* lSolidTemp = new G4UnionSolid("solid1", lTorusSolid2, lTorusSolid1, 0, lCenterOfPolycone);
   G4UnionSolid* lSolid = new G4UnionSolid("solid", lSolidTemp, lSphereSolid, 0, lCenterOfSphereUp);


   G4RotationMatrix* rot = new G4RotationMatrix();
   rot->rotateY(180.0 * deg);
   G4UnionSolid* lDEggShapeSolid = new G4UnionSolid("degg", lSolid, lSolid, rot, G4ThreeVector(0,0,0));

   return lDEggShapeSolid;
}


