
/** 
 *  @todo  - Investigate why the gel does not reach the photocathode edge.
 *         - Should the photocathode edge end earlier? Back PMT shape further collides with 'standard' CAD cones.
 */
#include "OMSimDEGG.hh"
#include "OMSimDEGGHarness.hh"
#include "OMSimTools.hh"
#include "OMSimLogger.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4Sphere.hh>
#include <G4Polycone.hh>


DEGG::DEGG(G4bool p_placeHarness) : OMSimOpticalModule(new OMSimPMTConstruction()), m_placeHarness(p_placeHarness)
{
   log_info("Constructing D-Egg");
   m_managerPMT->selectPMT("pmt_Hamamatsu_R5912_20_100");
   m_managerPMT->construction();
    if (p_placeHarness) m_harness = new DEggHarness(this);
    construction();
    if (p_placeHarness) integrateDetectorComponent(m_harness, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "");
   log_trace("Finished constructing D-Egg");
}


/**
 * @brief Construction of the whole DEGG. If you want to change any component, you have to change it at the specific function.
 */
void DEGG::construction()
{
   // Variables used for creating the outer glass
   const G4double outSegment1 = 30;
   const G4double outSphereRadiusMax = 156.0 * mm;
   const G4double outSphereDtheta = 45.7191 * deg;
   const G4double outTransformZ = 110.0 * mm;
   const G4double outTorusRadius1 = 85.0 * mm;
   const G4double outCenterOfTorusRadius1 = 50.8307 * mm;
   const G4int outSegments2 = 40;
   const G4double outTorusRadius2 = 1000.0 * mm;
   const G4double outCenterOfTorusRadius2 = -850.0 * mm;
   const G4double outCenterOfTorusZ2 = 0.1676 * mm;
   const G4double outTorusZmin2 = 24.9334 * mm;
   const G4double outTorusZmax2 = 175.4714 * mm;
   const G4double outTorusZ0 = 151.0 * mm;
   const G4double outTorusTransformZ = 160.5706 * mm;

   // Variables used for creating the internal volume
   const G4int innSegments1 = 300;
   const G4double innSphereRadiusMax = 136.0 * mm;
   const G4double innSphereDtheta = 46.7281 * deg;
   const G4double innTransformZ = 121.0 * mm;
   const G4double innTorusRadius1 = 65.0 * mm;
   const G4double innCenterOfTorusRadius1 = 51.3850 * mm;
   const G4int innSegments2 = 200;
   const G4double innTorusRadius2 = 1150.0 * mm;
   const G4double innCenterOfTorusRadius2 = -1019.9992 * mm;
   const G4double innCenterOfTorusZ2 = -1.3972 * mm;
   const G4double innTorusZmin2 = 6.00909 * mm;
   const G4double innTorusZmax2 = 180.2726 * mm;
   const G4double innTorusZ0 = 130.0 * mm;
   const G4double innTorusTransformZ = 170.7198 * mm;

   // Variable used for creating the subtraction box
   const G4double gelHeight = 180.5 * mm;

   // Create pressure vessel and inner volume
   G4VSolid *outerGlass = createEggSolid(outSegment1,
                                         outSphereRadiusMax,
                                         outSphereDtheta,
                                         outTransformZ,
                                         outTorusRadius1,
                                         outCenterOfTorusRadius1,
                                         outSegments2,
                                         outTorusRadius2,
                                         outCenterOfTorusRadius2,
                                         outCenterOfTorusZ2,
                                         outTorusZmin2,
                                         outTorusZmax2,
                                         outTorusZ0,
                                         outTorusTransformZ);

   G4VSolid *internalVolume = createEggSolid(innSegments1,
                                             innSphereRadiusMax,
                                             innSphereDtheta,
                                             innTransformZ,
                                             innTorusRadius1,
                                             innCenterOfTorusRadius1,
                                             innSegments2,
                                             innTorusRadius2,
                                             innCenterOfTorusRadius2,
                                             innCenterOfTorusZ2,
                                             innTorusZmin2,
                                             innTorusZmax2,
                                             innTorusZ0,
                                             innTorusTransformZ);

   

   // Make box to substract empty space
   G4Box *lSubstractionBox = new G4Box("SubstractionBox", 20 * cm, 20 * cm, gelHeight);
   G4LogicalVolume *lLogicalDummy = new G4LogicalVolume(lSubstractionBox, m_data->getMaterial("Ri_Air"), "Temp");

   // Append all internal components
   appendComponent(lSubstractionBox, lLogicalDummy, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "SubstractionBox");

   // Substract all internal components to internal volume to obtain gel and append it
   G4VSolid *lGelLayers = substractToVolume(internalVolume, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "DeggGelLayersSolid");

   // Delete dummy box from internal components
   deleteComponent("SubstractionBox");

   //Internal components
   G4RotationMatrix lRotationInternal;
   G4ThreeVector lOriginInternal(-179 * mm, 34.15 * mm, 32.25 * mm);
   Tools::AppendCADComponent(this, 1.0, lOriginInternal, lRotationInternal, "DEGG/Internal_modified.obj", "CAD_Internal", m_data->getMaterial("NoOptic_Stahl"), m_aluVis,  m_data->getOpticalSurface("Surf_StainlessSteelGround"));
   //G4ThreeVector lOriginInternal(-427.6845 * mm, 318.6396 * mm, 154 * mm); //files below are original but have overlaps with PMT
   //Tools::AppendCADComponent(this, 1.0, lOriginInternal, lRotationInternal, "DEGG/Internal_OnlyCones.obj", "CAD_Internal", m_data->getMaterial("NoOptic_Absorber"), m_aluVis);
   //Tools::AppendCADComponent(this, 1.0, lOriginInternal, lRotationInternal, "DEGG/Internal_Everything_NoMainboard.obj", "CAD_Internal", m_data->getMaterial("NoOptic_Absorber"), m_aluVis);
   
   // Logicals
   G4LogicalVolume *lDEggGlassLogical = new G4LogicalVolume(outerGlass, m_data->getMaterial("RiAbs_Glass_Okamoto_DOUMEKI"), "Glass_log");
   G4LogicalVolume *lInnerVolumeLogical = new G4LogicalVolume(internalVolume, m_data->getMaterial("Ri_Air"), "InnerVolume");


   //subtract PCA
   if (m_placeHarness)
    {
        lInnerVolumeLogical = new G4LogicalVolume(substractHarnessPCA(internalVolume),
                                         m_data->getMaterial("Ri_Air"),
                                         "InnerVolume");
        lDEggGlassLogical = new G4LogicalVolume(substractHarnessPCA(outerGlass),
                                           m_data->getMaterial("RiAbs_Glass_Okamoto_DOUMEKI"),
                                           "Glass_log");
    }

   // place internal volume in glass
   new G4PVPlacement(new G4RotationMatrix(), G4ThreeVector(0, 0, 0), lInnerVolumeLogical, "VacuumGlass", lDEggGlassLogical, false, 0, m_checkOverlaps);

   // Place PMTs into that volume
   lGelLayers = placePMTs(lInnerVolumeLogical, lGelLayers);
   G4LogicalVolume *lGelLogical = new G4LogicalVolume(lGelLayers, m_data->getMaterial("RiAbs_Gel_Shin-Etsu"), "DeggGelLayersLogical");
   appendComponent(lGelLayers, lGelLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "DeggGelLayers");

   // Placements
   // Place all internal components in internal volume
   placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), lInnerVolumeLogical, "");
   // Delete all internal components from dictionary, as they were placed in a volume inside the largest volume.
   m_components.clear();



   // Add glass volume to component map
   appendComponent(outerGlass, lDEggGlassLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PressureVessel_" + std::to_string(m_index));

   // ---------------- visualisation attributes --------------------------------------------------------------------------------
   lDEggGlassLogical->SetVisAttributes(m_glassVis);
   lInnerVolumeLogical->SetVisAttributes(m_airVis);
   lGelLogical->SetVisAttributes(m_gelVis);
   lLogicalDummy->SetVisAttributes(m_airVis);
}

G4VSolid *DEGG::placePMTs(G4LogicalVolume *p_innerVolume, G4VSolid *p_gelLayer)
{
   G4double distancePMT = 176.7 * mm;
   for (int k = 0; k <= 2 - 1; k++)
   {
      m_converter.str("");
      m_converter << "_" << k;

      G4RotationMatrix *rot2 = new G4RotationMatrix();

      if (k == 1)
      {
         rot2->rotateY(180 * deg);
      }

      G4Transform3D transformers = G4Transform3D(*rot2, G4ThreeVector(0, 0, distancePMT * std::pow(-1, k)));
      m_managerPMT->placeIt(transformers, p_innerVolume, m_converter.str());

      p_gelLayer = new G4SubtractionSolid("SubstractedVolume", p_gelLayer, m_managerPMT->getPMTSolid(), transformers);
   }
   return p_gelLayer;
}

/**
 * @brief Creates the solid shape for the DEGG pressure vessel.
 * @param p_segments1 Number of segments for the G4Sphere representing the outer glass.
 * @param p_sphereRmax Outer radius of the G4Sphere representing the outer glass.
 * @param p_spheredTheta Delta theta angle of the G4Sphere segment.
 * @param p_sphereTransformZ Shift of the G4Sphere in the z-direction.
 * @param p_torus1R Radius of the small spindle torus sphere.
 * @param p_centreOfTorus1R Distance from the center of torus 1 to the z-axis.
 * @param p_segments2 Number of segments for the large spindle torus sphere.
 * @param p_torus2R Radius of the large spindle torus sphere.
 * @param p_centreOfTorus2R Distance from the center of torus 2 to the z-axis (signed).
 * @param p_centreOfTorus2z Distance from the center of torus 2 to the z-axis (signed).
 * @param p_torus2Zmin Minimum z shift from z=0 in the positive z direction.
 * @param p_torus2Zmax Maximum z shift from z=0 in the positive z direction.
 * @param p_torus2Z0 G4double.
 * @param p_torus1TransformZ G4double.
 * @return The outer or inner shape of the glass vessel as a G4VSolid.
 */
G4VSolid *DEGG::createEggSolid(G4int p_segments1,
                               G4double p_sphereRmax,
                               G4double p_spheredTheta,
                               G4double p_sphereTransformZ,
                               G4double p_torus1R,
                               G4double p_centreOfTorus1R,
                               G4int p_segments2,
                               G4double p_torus2R,
                               G4double p_centreOfTorus2R,
                               G4double p_centreOfTorus2z,
                               G4double p_torus2Zmin,
                               G4double p_torus2Zmax,
                               G4double p_torus2Z0,
                               G4double p_torus1TransformZ)
{

   // Create Egg sphere
   G4Sphere *sphereSolid = new G4Sphere("sphere", 0, p_sphereRmax, 0. * degree, 2 * M_PI, 0. * degree, p_spheredTheta);
   G4ThreeVector centerOfSphereUp(0, 0, p_sphereTransformZ);

   // Torus Part 1
   // buidling small polycones, define full size of torus

   G4double rInner[p_segments1 + 1], rOuter[p_segments1 + 1], zPlane[p_segments1 + 1];
   G4double step = p_torus1R / p_segments1;
   std::vector<G4double> temporalZ, temporalOuter;

   G4double r;
   for (G4int j = 0; j <= p_segments1; ++j)
   {
      r = sqrt((2 * p_torus1R - j * step) * j * step);
      zPlane[j] = p_torus1R - j * step;
      rInner[j] = 0.;
      rOuter[j] = p_centreOfTorus1R + r;
   }

   G4Polycone *torusSolid1 = new G4Polycone("torus1", 0, 2 * M_PI, p_segments1 + 1, zPlane, rInner, rOuter);

   // Torus Part 2
   // Building large sphere revolution
   G4double rInner2[p_segments2 + 1], rOuter2[p_segments2 + 1], zPlane2[p_segments2 + 1];

   G4double zMinRelative = p_torus2Zmin - p_centreOfTorus2z; // minimum z shift from center of torus in positive z direction
   G4double zMaxRelative = p_torus2Zmax - p_centreOfTorus2z; // maximum z shift from center of torus in positive z direction
   step = (zMaxRelative - zMinRelative) / (p_segments2 - 1);

   G4double relativeZMax2 = p_torus2Zmax - p_centreOfTorus2z;
   for (G4int j = 0; j <= p_segments2 - 1; ++j)
   {
      rInner2[j] = 0;
      r = sqrt((p_torus2R + relativeZMax2 - j * step) * (p_torus2R - relativeZMax2 + j * step));
      zPlane2[j] = p_torus2Zmax - j * step;
      rOuter2[j] = p_centreOfTorus2R + r;
   }
   rInner2[p_segments2] = 0;
   zPlane2[p_segments2] = 0.;
   rOuter2[p_segments2] = p_torus2Z0;

   G4Polycone *torusSolid2 = new G4Polycone("polycone2", 0, 2 * M_PI, p_segments2 + 1, zPlane2, rInner2, rOuter2);

   // Create Vessel

   G4ThreeVector centreOfPolycone(0, 0, p_torus1TransformZ);

   G4UnionSolid *temporalSolid = new G4UnionSolid("solid1", torusSolid2, torusSolid1, 0, centreOfPolycone);
   G4UnionSolid *solid = new G4UnionSolid("solid", temporalSolid, sphereSolid, 0, centerOfSphereUp);

   G4RotationMatrix *rot = new G4RotationMatrix();
   rot->rotateY(180.0 * deg);
   G4UnionSolid *shapeSolidDegg = new G4UnionSolid("degg", solid, solid, rot, G4ThreeVector(0, 0, 0));
   return shapeSolidDegg;
}

G4SubtractionSolid *DEGG::substractHarnessPCA(G4VSolid *p_solid)
{
    Component plug = m_harness->getComponent("CAD_PCA");
    G4Transform3D plugTransform = G4Transform3D(plug.Rotation, plug.Position);
    G4SubtractionSolid *solidSubstracted = new G4SubtractionSolid(p_solid->GetName() + "_PCASubstracted", p_solid, plug.VSolid, plugTransform);
    return solidSubstracted;
}