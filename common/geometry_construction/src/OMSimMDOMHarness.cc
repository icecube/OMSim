/** 
 * @todo - Write documentation and parse current comments into Doxygen style
 */
#include "OMSimMDOMHarness.hh"
#include "OMSimMDOM.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4EllipticalTube.hh>
#include <G4Sphere.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4Polycone.hh>
#include <G4Torus.hh>

mDOMHarness::mDOMHarness(mDOM *p_MDOM): OMSimDetectorComponent(), m_opticalModule(p_MDOM)
{
    m_totalWidth = m_opticalModule->m_glassOutRad + m_teraThickness + m_padThickness + m_bridgeAddedThickness;
    construction();
};

void mDOMHarness::construction()
{

    bandsAndClamps();
    // bridgeRopesSolid();
    mainDataCable();
    pads();
    PCA();
    plug();
    teraBelt();
}

void mDOMHarness::bandsAndClamps()
{
    // vertical band dimensions
    G4double bandThickness = 0.5 * mm; //(taken from construction sketch of the Band w/o Penetrator provided by Anna Pollmann)
    G4double bandWidth = 9.0 * mm;     //(taken from construction sketch of the Band w/o Penetrator provided by Anna Pollmann)

    // clamp dimensions
    G4double clampLenght = 10.0 * mm; // according to Fusion 360 model of the harness provided by Anna Pollmann

    // First band
    G4Tubs *bandASolid = new G4Tubs("bandASolid", m_opticalModule->m_glassOutRad, m_opticalModule->m_glassOutRad + bandThickness, bandWidth / 2, 0, CLHEP::pi);

    // Second band, this is the band with the ring for the penetrator
    G4Tubs *bandTempSolid = new G4Tubs("lBandBSolid_temp", m_opticalModule->m_glassOutRad - 1. * cm, m_opticalModule->m_glassOutRad + bandThickness + 1. * cm, bandWidth / 2 + 1. * cm, 90. * deg - m_plugAngle - 10. * deg, 20. * deg);
    G4SubtractionSolid *bandNoRingSolid = new G4SubtractionSolid("bandNoRingSolid", bandASolid, bandTempSolid);

    // This band's ring.. the ring to rule them all!
    G4RotationMatrix tempTubeRotation = G4RotationMatrix();
    tempTubeRotation.rotateY(90. * deg);
    tempTubeRotation.rotateX(90. * deg);
    tempTubeRotation.rotateZ(90. * deg - m_plugAngle);
    G4EllipticalTube *tempTubeOuterSolid = new G4EllipticalTube("tempTubeOuterSolid", 35. * mm, 24.5 * mm, 2000. * mm);
    G4EllipticalTube *tempTubeInnerSolid = new G4EllipticalTube("tempTubeInnerSolid", 25. * mm, 17.49 * mm, 2500. * mm);
    G4SubtractionSolid *tempTubeSolid = new G4SubtractionSolid("tempTubeSolid", tempTubeOuterSolid, tempTubeInnerSolid);
    G4Sphere *tempSphereSolid = new G4Sphere("tempSphereSolid", m_opticalModule->m_glassOutRad, m_opticalModule->m_glassOutRad + bandThickness, 0, CLHEP::pi, 0, CLHEP::pi);

    G4IntersectionSolid *ringSolid = new G4IntersectionSolid("ringSolid", tempSphereSolid, tempTubeSolid, G4Transform3D(tempTubeRotation, G4ThreeVector(0, 0, 0)));
    // Now we union the ring with the band, which should be already aligned
    G4RotationMatrix bandRotation = G4RotationMatrix();
    G4UnionSolid *bandSolid = new G4UnionSolid("bandSolid", bandNoRingSolid, ringSolid, &bandRotation, G4ThreeVector(0, 0, 0));

    // Separate bands into 2 different solids because of weird visualizacion problems
    bandRotation = G4RotationMatrix();
    bandRotation.rotateY(90 * deg);
    bandRotation.rotateZ(180 * deg);
    bandRotation.rotateX(180 * deg);

    G4UnionSolid *lBandsSolidTop = new G4UnionSolid("Band_solid_top", bandASolid, bandSolid, &bandRotation, G4ThreeVector(0, 0, 0));

    // bottom bands, this is twice bandASolid
    G4UnionSolid *bandSolidBottom = new G4UnionSolid("Band_solid_bottom", bandASolid, bandASolid, &bandRotation, G4ThreeVector(0, 0, 0));

    G4LogicalVolume *bandLogicalTop = new G4LogicalVolume(lBandsSolidTop, m_data->getMaterial("NoOptic_Stahl"), "mBands logical top");
    G4LogicalVolume *bandLogicalBottom = new G4LogicalVolume(bandSolidBottom, m_data->getMaterial("NoOptic_Stahl"), "mBands logical bottom");

    new G4LogicalSkinSurface("band_skin", bandLogicalTop, m_data->getOpticalSurface("Surf_StainlessSteelGround"));
    new G4LogicalSkinSurface("band_skin", bandLogicalBottom, m_data->getOpticalSurface("Surf_StainlessSteelGround"));

    bandLogicalTop->SetVisAttributes(m_steelVis);
    bandLogicalBottom->SetVisAttributes(m_steelVis);

    G4RotationMatrix bandRotationTop = G4RotationMatrix();
    bandRotationTop.rotateX(90 * deg);
    bandRotationTop.rotateZ(-m_harnessRotAngle);
    G4ThreeVector bandPosition = G4ThreeVector(0, 0, m_opticalModule->m_cylinderHeight + 1. * mm);
    appendComponent(lBandsSolidTop, bandLogicalTop, bandPosition, bandRotationTop, "Band_Top");

    G4RotationMatrix bandRotationBottom = G4RotationMatrix();
    bandRotationBottom.rotateX(-90 * deg);
    bandRotationBottom.rotateZ(-m_harnessRotAngle);
    appendComponent(bandSolidBottom, bandLogicalBottom, -bandPosition, bandRotationBottom, "Band_Bottom");
}

void mDOMHarness::bridgeRopesSolid()
{
    const G4double bridgePhiStart = 83.0 * deg;
    const G4double bridgeTotalPhi = 14.0 * deg;
    const G4double bridgeRinner = m_opticalModule->m_glassOutRad + m_teraThickness + m_padThickness;
    const G4double bridgeRInnerList[6] = {bridgeRinner, bridgeRinner, bridgeRinner, bridgeRinner, bridgeRinner, bridgeRinner};

    const G4double bridgeROuterTemp[6] = {(bridgeRinner + 2.2 * mm),
                                           (bridgeRinner + 7.180628394561269 * mm),
                                           (bridgeRinner + 12.255684206223851 * mm),
                                           (bridgeRinner + 12.255684206223851 * mm),
                                           (bridgeRinner + 7.180628394561269 * mm),
                                           (bridgeRinner + 2.2 * mm)};

    const G4double bridgeROuter[6] = {bridgeROuterTemp[0],
                                       (bridgeROuterTemp[0] + 10.83279768266868 * mm),
                                       m_totalWidth,
                                       m_totalWidth,
                                       (bridgeROuterTemp[0] + 10.83279768266868 * mm),
                                       bridgeROuterTemp[0]};

    const G4double bridgeZPlane[6] = {-27.0 * mm,
                                       -16.16720231733132 * mm,
                                       -7.85 * mm,
                                       7.85 * mm,
                                       16.16720231733132 * mm,
                                       27.0 * mm};

    const G4double bridgeZTemp[6] = {-17.0 * mm,
                                      -14.879793181282428 * mm,
                                      -7.85 * mm,
                                      7.85 * mm,
                                      14.879793181282428 * mm,
                                      17.0 * mm};

    G4Polycone *bridgeTemp1Solid = new G4Polycone("BridgeSolid", bridgePhiStart, bridgeTotalPhi, 6, bridgeZPlane, bridgeRInnerList, bridgeROuter);
    G4Polycone *bridgeTemp2Solid = new G4Polycone("bridgeTemp2Solid", bridgePhiStart - 1.0 * deg, 16.0 * deg, 6, bridgeZTemp, bridgeRInnerList, bridgeROuterTemp);
    G4SubtractionSolid *bridgeSolid = new G4SubtractionSolid("", bridgeTemp1Solid, bridgeTemp2Solid);

    const G4double ropesZShift = 2.0 * mm;

    G4Tubs *ropesSolid = new G4Tubs("ropesSolid", 0.0, m_ropeRMax, m_ropeDz - ropesZShift, 0.0, 360.0 * deg);

    // First union
    G4RotationMatrix rot = G4RotationMatrix();
    rot.rotateX(m_ropeRotationAngleX);

    G4ThreeVector unionPos = G4ThreeVector(0, m_opticalModule->m_glassOutRad - m_ropeDz * sin(m_ropeRotationAngleX) + 2.25 * cm, (m_ropeDz)*cos(m_ropeRotationAngleX));
    G4UnionSolid *tempUnion1 = new G4UnionSolid("temp1", bridgeSolid, ropesSolid, G4Transform3D(rot, unionPos));

    // second union, rope in the other side
    rot = G4RotationMatrix();
    rot.rotateX(-m_ropeRotationAngleX);
    unionPos = G4ThreeVector(0, m_opticalModule->m_glassOutRad - m_ropeDz * sin(m_ropeRotationAngleX) + 2.25 * cm, -(m_ropeDz)*cos(m_ropeRotationAngleX));
    G4UnionSolid *lTempUnion2 = new G4UnionSolid("temp2", tempUnion1, ropesSolid, G4Transform3D(rot, unionPos));

    // and now this same object 3 more times, with 90 degrees difference
    rot = G4RotationMatrix();
    rot.rotateZ(90 * deg);
    G4UnionSolid *tempUnion3 = new G4UnionSolid("temp3", lTempUnion2, lTempUnion2, G4Transform3D(rot, G4ThreeVector(0, 0, 0)));
    rot.rotateZ(90 * deg);
    G4UnionSolid *tempUnion4 = new G4UnionSolid("temp4", tempUnion3, lTempUnion2, G4Transform3D(rot, G4ThreeVector(0, 0, 0)));
    rot.rotateZ(90 * deg);

    G4UnionSolid *ropesUnionSolid = new G4UnionSolid("ropesUnionSolid", tempUnion4, lTempUnion2, G4Transform3D(rot, G4ThreeVector(0, 0, 0)));
    G4LogicalVolume *ropesUnionLogical = new G4LogicalVolume(ropesUnionSolid, m_data->getMaterial("NoOptic_Stahl"), "");

    new G4LogicalSkinSurface("ropes_skin", ropesUnionLogical, m_data->getOpticalSurface("Surf_StainlessSteelGround"));

    G4RotationMatrix ropesRot = G4RotationMatrix();
    ropesRot.rotateZ(m_harnessRotAngle);

    appendComponent(ropesUnionSolid, ropesUnionLogical, G4ThreeVector(0, 0, 0), ropesRot, "Ropes");
}

void mDOMHarness::mainDataCable()
{
    const G4double dataCableRadius = 26.0 * mm; // Radius of the main data cable (according to Prof. Kappes)
    const G4double dataCableLength = 4.0 * m;   // Length of main data cable

    G4Tubs *dataCableSolid = new G4Tubs("MainDataCable_solid", 0, dataCableRadius, dataCableLength / 2.0, 0, 2 * CLHEP::pi);

    G4LogicalVolume *dataCableLogical = new G4LogicalVolume(dataCableSolid, m_data->getMaterial("NoOptic_Absorber"), "MainDataCable_logical");
    new G4LogicalSkinSurface("MainDataCable_skin", dataCableLogical, m_data->getOpticalSurface("Surf_BlackDuctTapePolished"));
    dataCableLogical->SetVisAttributes(m_absorberVis);

    G4double mainCableAngle = 90.0 * deg;
    G4double mainCableRadius = (m_ropeStartingPoint + dataCableRadius + m_ropeRMax + 0.2 * cm - m_bridgeAddedThickness - 0.75 * cm);
    G4ThreeVector dataCablePosition = G4ThreeVector(mainCableRadius * sin(mainCableAngle),
                                                     mainCableRadius * cos(mainCableAngle),
                                                     0.0);

    appendComponent(dataCableSolid, dataCableLogical, dataCablePosition, G4RotationMatrix(), "mainDataCable");
}

void mDOMHarness::pads()
{
    const G4double padWidth = 11.0 * mm;  // (taken from construction sketch of the rubber pads provided by Anna Pollmann)
    const G4double padAngle = 44.5 * deg; // (taken from construction sketch of the rubber pads provided by Anna Pollmann)

    G4Tubs *padSolid = new G4Tubs("padSolid", m_opticalModule->m_glassOutRad + m_teraThickness, m_opticalModule->m_glassOutRad + m_teraThickness + m_padThickness, padWidth / 2.0, padAngle / 2.0, padAngle);

    G4RotationMatrix rot = G4RotationMatrix();

    G4UnionSolid *tempPadUnion1 = new G4UnionSolid("temppad1", padSolid, padSolid, G4Transform3D(rot, G4ThreeVector(0, 0, (-44.0) * mm)));
    rot.rotateZ(90.0 * deg);
    G4UnionSolid *tempPadUnion2 = new G4UnionSolid("temppad2", tempPadUnion1, tempPadUnion1, G4Transform3D(rot, G4ThreeVector(0, 0, 0)));
    rot.rotateZ(90.0 * deg);
    G4UnionSolid *tempPadUnion3 = new G4UnionSolid("temppad3", tempPadUnion2, tempPadUnion1, G4Transform3D(rot, G4ThreeVector(0, 0, 0)));
    rot.rotateZ(90.0 * deg);

    G4UnionSolid *padUnionSolid = new G4UnionSolid("padUnionSolid", tempPadUnion3, tempPadUnion1, G4Transform3D(rot, G4ThreeVector(0, 0, 0)));
    G4LogicalVolume *padUnionLogical = new G4LogicalVolume(padUnionSolid, m_data->getMaterial("NoOptic_Absorber"), "");
    new G4LogicalSkinSurface("pad_skin", padUnionLogical, m_data->getOpticalSurface("Surf_PadSurface"));

    padUnionLogical->SetVisAttributes(m_redVis);

    G4RotationMatrix padRot = G4RotationMatrix();
    padRot.rotateZ(m_harnessRotAngle - 14.5 * deg);
    appendComponent(padUnionSolid, padUnionLogical, G4ThreeVector(0, 0, (22.0) * mm), padRot, "pads");
}

void mDOMHarness::PCA()
{
    // Cylindrical metallic part
    const G4double mushRadiusUpper = 10.965 * mm; // Radius of the (black) conical part of the penetrator at the thicker end (measured by hand)
    const G4double mushRadiusLower = 15.0 * mm;   // Radius of the (black) conical part of the penetrator at the thinner end (according to construction sketch by Hydrogroup)
    const G4double mushHeight = 32.77 * mm;       // Height of the penetrator part that is made up of a conical and a cylindrical part
    const G4double mushCylinderHeight = 6.35 * mm;     // Height of cylindrical part (measured by hand)
    const G4double mushConeHeight = mushHeight - mushCylinderHeight;

    G4Tubs *mushCylinderSolid = new G4Tubs("Mush solid", 0, mushRadiusLower, mushCylinderHeight / 2., 0, 2. * CLHEP::pi);
    G4LogicalVolume *mushCylinderLogical = new G4LogicalVolume(mushCylinderSolid, m_data->getMaterial("NoOptic_Stahl"), "Mush logical");
    new G4LogicalSkinSurface("mush_cyl_skin", mushCylinderLogical, m_data->getOpticalSurface("Surf_AluminiumGround"));
    mushCylinderLogical->SetVisAttributes(m_steelVis);

    G4RotationMatrix mushRot = G4RotationMatrix();
    mushRot.rotateX(-m_plugAngle);
    mushRot.rotateZ(m_harnessRotAngle - 90 * deg);
    G4ThreeVector mushPosition = G4ThreeVector((m_opticalModule->m_glassOutRad + mushCylinderHeight / 2) * (cos(m_harnessRotAngle)) * sin(m_plugAngle), (m_opticalModule->m_glassOutRad + mushCylinderHeight / 2) * sin(m_harnessRotAngle) * sin(m_plugAngle), (m_opticalModule->m_glassOutRad + mushCylinderHeight / 2) * cos(m_plugAngle) + m_opticalModule->m_cylinderHeight);
    appendComponent(mushCylinderSolid, mushCylinderLogical, mushPosition, mushRot, "Mush");

    // Now building the hole penetrator cable as a single entity
    // first the conical cap
    G4Cons *mushConeSolid = new G4Cons("mush_cone_solid", 0, mushRadiusLower, 0, mushRadiusUpper, mushConeHeight / 2., 0, 2. * CLHEP::pi);

    // rigid part of the penetrator plastic, here it is called tube?
    const G4double tubeRadius = 10.67 * mm; // Radius of the rigid part of the cable that exits the penetrator part (according to construction sketch by Hydrogroup)
    const G4double tubeLength = 60.0 * mm;  // Length of the rigid part of the cable that exits the penetrator part (measured by hand)
    G4Tubs *tubeSolid = new G4Tubs("tube_solid", 0, tubeRadius, tubeLength / 2, 0, 2 * CLHEP::pi);

    G4RotationMatrix rot;
    rot.rotateX(90 * deg);
    G4Transform3D unionTransform = G4Transform3D(rot, G4ThreeVector(0, -tubeLength / 2., mushConeHeight / 2. - tubeRadius));

    G4UnionSolid *penCableRigidSolid = new G4UnionSolid("Penetrator cable solid rigid part", mushConeSolid, tubeSolid, unionTransform);

    // Building PCA. This is gonna be built from outside to the inside, to do the unions easily
    // variables
    const G4double wireThickness = 1.3716 * cm;                                                                                                                                                                                                                                                                                                                                                                                                                                                   // south bay cable width (0.54 inches) (according to construction sketch by South Bay Corporation)
    const G4double wireRadius = 17.78 * cm;                                                                                                                                                                                                                                                                                                                                                                                                                                                       // minimum bending diameter of the cable (14 inches)
    const G4double disanceTubeToRope = ((m_ropeStartingPoint) + m_ropeRMax * (1 - cos(m_ropeRotationAngleX)) - ((m_opticalModule->m_glassOutRad + mushHeight - tubeRadius) * cos(m_plugAngle) + tubeLength * sin(m_plugAngle) + m_opticalModule->m_cylinderHeight) * tan(m_ropeRotationAngleX) - (m_opticalModule->m_glassOutRad + mushHeight - tubeRadius) * sin(m_plugAngle) + tubeLength * cos(m_plugAngle)) / cos(m_ropeRotationAngleX) - (wireThickness / 2 + m_ropeRMax) - m_ropeDz * (1 - cos(m_ropeRotationAngleX)) * sin(m_ropeRotationAngleX); // Modified distance between start of first torus and end of second torus

    const G4double anglePCA2 = acos((0.5) * (disanceTubeToRope / wireRadius + sin(m_plugAngle + m_ropeRotationAngleX) + 1.));
    const G4double anglePCA1 = 90. * deg + anglePCA2 - (m_plugAngle + m_ropeRotationAngleX);
    const G4double extnLength = 30. * 2.54 * cm - wireRadius * (anglePCA1 + anglePCA2) - tubeLength;

    // PCA1
    G4Torus *solidPCA1 = new G4Torus("lPCA1Solid", 0, wireThickness / 2., wireRadius, 0, anglePCA1);
    // PCA2
    G4Torus *solidPCA2 = new G4Torus("lPCA2Solid", 0, wireThickness / 2., wireRadius, 0, anglePCA2);
    // Straight part of the cable
    G4Tubs *extnSolid = new G4Tubs("lExtnSolid", 0, wireThickness / 2., extnLength / 2., 0, 2 * CLHEP::pi);

    // Now first union
    G4ThreeVector pos1 = G4ThreeVector(wireRadius, -extnLength / 2., 0);
    G4UnionSolid *cableTempUnion1 = new G4UnionSolid("temp1", solidPCA2, extnSolid, G4Transform3D(rot, pos1));

    // Second union
    G4RotationMatrix rot2 = G4RotationMatrix();
    rot2.rotateZ(-anglePCA2); // PCA2 ANGLE
    rot2.rotateY(180 * deg);
    G4ThreeVector pos2 = G4ThreeVector(2 * wireRadius, 0, 0);
    G4UnionSolid *cableTempUnion2 = new G4UnionSolid("temp2", solidPCA1, cableTempUnion1, G4Transform3D(rot2, pos2));

    // Now we union the cable with the rigid part
    G4RotationMatrix rot3 = G4RotationMatrix();
    rot3.rotateY(90. * deg);
    rot3.rotateX(-anglePCA1);
    G4ThreeVector pos3 = G4ThreeVector(0, -tubeLength, wireRadius + mushConeHeight / 2. - tubeRadius); // mushConeHeight/2.-tubeRadius is the translation of the previous union indeed

    G4UnionSolid *solidPCA = new G4UnionSolid("PCACable_solid", penCableRigidSolid, cableTempUnion2, G4Transform3D(rot3, pos3)); // reference frame of this object is the same as placing mushConeSolid alone, so the part in contact with the metal part of the penetrator in the module
    G4LogicalVolume *logicalPCA = new G4LogicalVolume(solidPCA, m_data->getMaterial("NoOptic_Absorber"), "Penetrator cable logical");
    new G4LogicalSkinSurface("penetrator cable skin", logicalPCA, m_data->getOpticalSurface("Surf_BlackDuctTapePolished"));
    logicalPCA->SetVisAttributes(m_absorberVis);

    G4ThreeVector positionPCA = G4ThreeVector((m_opticalModule->m_glassOutRad + (2 * mushCylinderHeight + mushConeHeight) / 2.) * (cos(m_harnessRotAngle)) * sin(m_plugAngle), (m_opticalModule->m_glassOutRad + (2 * mushCylinderHeight + mushConeHeight) / 2) * sin(m_harnessRotAngle) * sin(m_plugAngle), (m_opticalModule->m_glassOutRad + (2 * mushCylinderHeight + mushConeHeight) / 2) * cos(m_plugAngle) + m_opticalModule->m_cylinderHeight);
    appendComponent(solidPCA, logicalPCA, positionPCA, mushRot, "PCA");
}

void mDOMHarness::plug()
{
    const G4double plugRadius = 8.0 * mm;  // radius of the screw part of the penetrator (according to construction sketch by Hydrogroup)
    const G4double plugLength = 25.0 * mm; // length of the screw part of the penetrator (according to construction sketch by Hydrogroup)

    G4Tubs *plugSolid = new G4Tubs("plug_solid_temp", 0, plugRadius, plugLength / 2, 0, 2 * CLHEP::pi);

    G4ThreeVector positionPlug = G4ThreeVector((m_opticalModule->m_glassOutRad - plugLength / 2.) * cos(m_harnessRotAngle) * sin(m_plugAngle),
                                                (m_opticalModule->m_glassOutRad - plugLength / 2.) * sin(m_harnessRotAngle) * sin(m_plugAngle),
                                                m_opticalModule->m_cylinderHeight + (m_opticalModule->m_glassOutRad - plugLength / 2) * cos(m_plugAngle));
    G4RotationMatrix rotationPlug = G4RotationMatrix();
    rotationPlug.rotateX(-m_plugAngle);
    rotationPlug.rotateZ(m_harnessRotAngle - 90 * deg);

    G4LogicalVolume *plugLogical = new G4LogicalVolume(plugSolid, m_data->getMaterial("NoOptic_Stahl"), "Plug_logical");
    new G4LogicalSkinSurface("plug_skin", plugLogical, m_data->getOpticalSurface("Surf_AluminiumGround"));
    plugLogical->SetVisAttributes(m_steelVis);

    appendComponent(plugSolid, plugLogical, positionPlug, rotationPlug, "Plug");
}

void mDOMHarness::teraBelt()
{
    const G4double teraWidth = 50.0 * mm; // replace with the actual value
    G4double zCorners[] = {teraWidth / 2, 0, -teraWidth / 2};
    G4double R = m_opticalModule->m_glassOutRad + 0.8 * mm; // add something so they do not touch
    G4double rCornersInn[] = {R, R + (teraWidth / 2) * sin(m_opticalModule->m_cylinderAngle), R};
    R = m_opticalModule->m_glassOutRad + m_teraThickness;
    G4double rCornersOut[] = {R, R + (teraWidth / 2) * sin(m_opticalModule->m_cylinderAngle), R};

    G4Polycone *teraSolidTemporal1 = new G4Polycone("tera_solid_temp1", 0, 2 * CLHEP::pi, 3, rCornersInn, zCorners);
    G4Polycone *teraSolidTemporal2 = new G4Polycone("tera_solid_temp2", 0, 2 * CLHEP::pi, 3, rCornersOut, zCorners);
    G4SubtractionSolid *teraSolid = new G4SubtractionSolid("tera_solid", teraSolidTemporal2, teraSolidTemporal1, G4Transform3D(G4RotationMatrix(), G4ThreeVector(0, 0, 0)));

    G4LogicalVolume *teraLogical = new G4LogicalVolume(teraSolid, m_data->getMaterial("NoOptic_Absorber"), "tera_logical");
    new G4LogicalSkinSurface("tera_skin", teraLogical, m_data->getOpticalSurface("Surf_BlackDuctTapePolished"));
    teraLogical->SetVisAttributes(m_absorberVis);

    appendComponent(teraSolid, teraLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "TeraTapeBelt");
}
