/**
 * @todo
 *       - I am not sure if the harness is correctly implemented!
 *       - Clean up the code.
 *       - Subtract CAD penetrator from the vessel.
 *       - Investigate why the gel does not reach the photocathode edge.
 *         (Should the photocathode edge end earlier? Gel, Vessel, and PMT shape are correct.)
 */
#include "OMSimDEGG.hh"
#include "CADMesh.hh"
#include "OMSimLogger.hh"

#include <G4Sphere.hh>
#include <G4Polycone.hh>


DEGG::DEGG(InputDataManager *pData, G4bool pPlaceHarness)
{
   log_info("Constructing DEGG");
   mData = pData;
   mPMTManager = new OMSimPMTConstruction(mData);
   mPMTManager->selectPMT("pmt_Hamamatsu_R5912_20_100");
   mPMTManager->construction();
   construction(); // always before harness, otherwise harness will be deleted :(
   /*
      if (pPlaceHarness) {
         mHarness = new DEggHarness(this, mData);
         integrateDetectorComponent(mHarness, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "");
      }*/
}


void DEGG::construction()
{
   // Variables used for creating the outer glass
   G4double lOutSegments1 = 30;
   G4double lOutSphereRadiusMax = 156.0 * mm;
   G4double lOutSphereDtheta = 45.7191 * deg;
   G4double lOutTransformZ = 110.0 * mm;
   G4double lOutTorusRadius1 = 85.0 * mm;
   G4double lOutCenterOfTorusRadius1 = 50.8307 * mm;
   G4int lOutSegments2 = 40;
   G4double lOutTorusRadius2 = 1000.0 * mm;
   G4double lOutCenterOfTorusRadius2 = -850.0 * mm;
   G4double lOutCenterOfTorusZ2 = 0.1676 * mm;
   G4double lOutTorusZmin2 = 24.9334 * mm;
   G4double lOutTorusZmax2 = 175.4714 * mm;
   G4double lOutTorusZ0 = 151.0 * mm;
   G4double lOutTorusTransformZ = 160.5706 * mm;

   // Variables used for creating the internal volume
   G4int lInnSegments1 = 300;
   G4double lInnSphereRadiusMax = 136.0 * mm;
   G4double lInnSphereDtheta = 46.7281 * deg;
   G4double lInnTransformZ = 121.0 * mm;
   G4double lInnTorusRadius1 = 65.0 * mm;
   G4double lInnCenterOfTorusRadius1 = 51.3850 * mm;
   G4int lInnSegments2 = 200;
   G4double lInnTorusRadius2 = 1150.0 * mm;
   G4double lInnCenterOfTorusRadius2 = -1019.9992 * mm;
   G4double lInnCenterOfTorusZ2 = -1.3972 * mm;
   G4double lInnTorusZmin2 = 6.00909 * mm;
   G4double lInnTorusZmax2 = 180.2726 * mm;
   G4double lInnTorusZ0 = 130.0 * mm;
   G4double lInnTorusTransformZ = 170.7198 * mm;

   // Variable used for creating the subtraction box
   G4double lGelHeight = 180.5 * mm;

   // Create pressure vessel and inner volume
   G4VSolid *lOuterGlass = createEggSolid(lOutSegments1,
                                          lOutSphereRadiusMax,
                                          lOutSphereDtheta,
                                          lOutTransformZ,
                                          lOutTorusRadius1,
                                          lOutCenterOfTorusRadius1,
                                          lOutSegments2,
                                          lOutTorusRadius2,
                                          lOutCenterOfTorusRadius2,
                                          lOutCenterOfTorusZ2,
                                          lOutTorusZmin2,
                                          lOutTorusZmax2,
                                          lOutTorusZ0,
                                          lOutTorusTransformZ);

   G4VSolid *lInternalVolume = createEggSolid(lInnSegments1,
                                              lInnSphereRadiusMax,
                                              lInnSphereDtheta,
                                              lInnTransformZ,
                                              lInnTorusRadius1,
                                              lInnCenterOfTorusRadius1,
                                              lInnSegments2,
                                              lInnTorusRadius2,
                                              lInnCenterOfTorusRadius2,
                                              lInnCenterOfTorusZ2,
                                              lInnTorusZmin2,
                                              lInnTorusZmax2,
                                              lInnTorusZ0,
                                              lInnTorusTransformZ);

   // Make box to substract empty space

   G4Box *lSubstractionBox = new G4Box("SubstractionBox", 20 * cm, 20 * cm, lGelHeight);
   G4LogicalVolume *lLogicalDummy = new G4LogicalVolume(lSubstractionBox, mData->getMaterial("Ri_Air"), "Temp");

   // Append all internal components
   appendComponent(lSubstractionBox, lLogicalDummy, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "SubstractionBox");
   appendPMTs();

   // Substract all internal components to internal volume to obtain gel and append it
   G4VSolid *lGelLayers = substractToVolume(lInternalVolume, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "DeggGelLayersSolid");
   G4LogicalVolume *lGelLogical = new G4LogicalVolume(lGelLayers, mData->getMaterial("RiAbs_Gel_Shin-Etsu"), "DeggGelLayersLogical");
   appendComponent(lGelLayers, lGelLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "DeggGelLayers");

   // Delete dummy box from internal components
   deleteComponent("SubstractionBox");
   // appendInternalComponentsFromCAD();

   // Logicals
   G4LogicalVolume *lDEggGlassLogical = new G4LogicalVolume(lOuterGlass, mData->getMaterial("RiAbs_Glass_Okamoto_DOUMEKI"), "Glass_phys");
   G4LogicalVolume *lInnerVolumeLogical = new G4LogicalVolume(lInternalVolume, mData->getMaterial("Ri_Air"), "InnerVolume");

   // Placements
   // place all internal components in internal volume
   placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), lInnerVolumeLogical, "");

   // place internal volume in glass
   new G4PVPlacement(new G4RotationMatrix(), G4ThreeVector(0, 0, 0), lInnerVolumeLogical, "VacuumGlass", lDEggGlassLogical, false, 0, mCheckOverlaps);

   // Delete all internal components from dictionary, as they were placed in a volume inside the largest volume.
   mComponents.clear();

   // Add glass volume to component map
   // appendComponent(lInternalVolume, lInnerVolumeLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "Internal");
   appendComponent(lOuterGlass, lDEggGlassLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PressureVessel");
   // appendPressureVesselFromCAD();

   // ---------------- visualisation attributes --------------------------------------------------------------------------------
   lDEggGlassLogical->SetVisAttributes(mGlassVis);
   lInnerVolumeLogical->SetVisAttributes(G4VisAttributes::GetInvisible());
}


void DEGG::appendPMTs()
{
   G4double lPMTdistance = 176.7 * mm;
   G4RotationMatrix lRot = G4RotationMatrix();
   lRot.rotateY(180 * deg);

   appendComponent(mPMTManager->getPMTSolid(),
                   mPMTManager->getLogicalVolume(),
                   G4ThreeVector(0, 0, lPMTdistance),
                   G4RotationMatrix(),
                   "PMT_1");

   appendComponent(mPMTManager->getPMTSolid(),
                   mPMTManager->getLogicalVolume(),
                   G4ThreeVector(0, 0, -lPMTdistance),
                   lRot,
                   "PMT_2");
}


void DEGG::appendInternalComponentsFromCAD()
{
   G4String lFilePath = "../common/data/CADmeshes/DEGG/Internal_Everything_NoMainboard.obj";
   G4double lCADScale = 1.0;
   G4double lInternalCAD_x = -427.6845 * mm;
   G4double lInternalCAD_y = 318.6396 * mm;
   G4double lInternalCAD_z = 154 * mm;
   // load mesh
   auto lMesh = CADMesh::TessellatedMesh::FromOBJ(lFilePath);

   G4ThreeVector lCADoffset = G4ThreeVector(lInternalCAD_x, lInternalCAD_y, lInternalCAD_z); // measured from CAD file since origin =!= Module origin
   lMesh->SetScale(lCADScale);
   lMesh->SetOffset(lCADoffset * lCADScale);

   // Place all of the meshes it can find in the file as solids individually.
   for (auto iSolid : lMesh->GetSolids())
   {
      G4LogicalVolume *lSupportStructureLogical = new G4LogicalVolume(iSolid, mData->getMaterial("NoOptic_Absorber"), "SupportStructureCAD_Logical");
      lSupportStructureLogical->SetVisAttributes(mAluVis);
      appendComponent(iSolid, lSupportStructureLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "SupportStructureCAD");
   }
}


void DEGG::appendPressureVesselFromCAD()
{
   G4String lFilePath = "../common/data/CADmeshes/DEGG/pressure_vessel_noPenetratorHole.obj";
   G4double lCADScale = 1.0;

   auto lMesh = CADMesh::TessellatedMesh::FromOBJ(lFilePath);

   G4ThreeVector lCADoffset = G4ThreeVector(0, 0, 0); // measured from CAD file since origin =!= Module origin
   lMesh->SetScale(lCADScale);
   lMesh->SetOffset(lCADoffset * lCADScale);

   G4RotationMatrix *lRot = new G4RotationMatrix();
   lRot->rotateX(180 * deg);

   // Place all of the meshes it can find in the file as solids individually.
   G4UnionSolid *lPressureVessel = new G4UnionSolid("CADPV", lMesh->GetSolids().at(0), lMesh->GetSolids().at(0), lRot, G4ThreeVector(0, -2 * 111 * mm, 0));

   G4LogicalVolume *lSupportStructureLogical = new G4LogicalVolume(lPressureVessel, mData->getMaterial("RiAbs_Glass_Okamoto_DOUMEKI"), "PressureVessel");
   lSupportStructureLogical->SetVisAttributes(mAluVis);

   G4RotationMatrix lRotNP = G4RotationMatrix();
   lRotNP.rotateX(90 * deg);
   lSupportStructureLogical->SetVisAttributes(mGlassVis);

   appendComponent(lPressureVessel, lSupportStructureLogical, G4ThreeVector(0, 0, 111 * mm), lRotNP, "PressureVessel");
}


G4VSolid *DEGG::createEggSolid(G4int pSegments_1,
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
   G4Sphere *lSphereSolid = new G4Sphere("sphere", 0, pSphereRmax, 0. * degree, 2 * M_PI, 0. * degree, pSpheredTheta);
   G4ThreeVector lCenterOfSphereUp(0, 0, pSphereTransformZ);

   // Torus Part 1
   // buidling small polycones, define full size of torus

   G4double lRInner[pSegments_1 + 1], lROuter[pSegments_1 + 1], lZPlane[pSegments_1 + 1];
   G4double lStep = pTorus1R / pSegments_1;
   std::vector<G4double> lTempZ, lTempOuter;

   G4double lR;
   for (G4int j = 0; j <= pSegments_1; ++j)
   {
      lR = sqrt((2 * pTorus1R - j * lStep) * j * lStep);
      lZPlane[j] = pTorus1R - j * lStep;
      lRInner[j] = 0.;
      lROuter[j] = pCenterOfTorus1R + lR;
   }

   G4Polycone *lTorusSolid1 = new G4Polycone("torus1", 0, 2 * M_PI, pSegments_1 + 1, lZPlane, lRInner, lROuter);

   // Torus Part 2
   // Building large sphere revolution
   G4double lRInner2[pSegments_2 + 1], lROuter2[pSegments_2 + 1], lZPlane2[pSegments_2 + 1];

   G4double lZMinRelative = pTorus2_Zmin - pCenterOfTorus2_z; // minimum z shift from center of torus in positive z direction
   G4double lZMaxRelative = pTorus2_Zmax - pCenterOfTorus2_z; // maximum z shift from center of torus in positive z direction
   lStep = (lZMaxRelative - lZMinRelative) / (pSegments_2 - 1);

   G4double lRelativeZMax2 = pTorus2_Zmax - pCenterOfTorus2_z;
   for (G4int j = 0; j <= pSegments_2 - 1; ++j)
   {
      lRInner2[j] = 0;
      lR = sqrt((pTorus2R + lRelativeZMax2 - j * lStep) * (pTorus2R - lRelativeZMax2 + j * lStep));
      lZPlane2[j] = pTorus2_Zmax - j * lStep;
      lROuter2[j] = pCenterOfTorus2R + lR;
   }
   lRInner2[pSegments_2] = 0;
   lZPlane2[pSegments_2] = 0.;
   lROuter2[pSegments_2] = pTorus2_Z0;

   G4Polycone *lTorusSolid2 = new G4Polycone("polycone2", 0, 2 * M_PI, pSegments_2 + 1, lZPlane2, lRInner2, lROuter2);

   // Create Vessel

   G4ThreeVector lCenterOfPolycone(0, 0, pTorus1TransformZ);

   G4UnionSolid *lSolidTemp = new G4UnionSolid("solid1", lTorusSolid2, lTorusSolid1, 0, lCenterOfPolycone);
   G4UnionSolid *lSolid = new G4UnionSolid("solid", lSolidTemp, lSphereSolid, 0, lCenterOfSphereUp);

   G4RotationMatrix *rot = new G4RotationMatrix();
   rot->rotateY(180.0 * deg);
   G4UnionSolid *lDEggShapeSolid = new G4UnionSolid("degg", lSolid, lSolid, rot, G4ThreeVector(0, 0, 0));

   return lDEggShapeSolid;
}
