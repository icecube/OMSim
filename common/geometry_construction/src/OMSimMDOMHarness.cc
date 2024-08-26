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

mDOMHarness::mDOMHarness(mDOM *pMDOM): abcDetectorComponent(), mOM(pMDOM)
{
    mTotalWidth = mOM->mGlassOutRad + mTeraThickness + mPadThickness + mBridgeAddedThickness;
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
    G4double lBandThickness = 0.5 * mm; //(taken from construction sketch of the Band w/o Penetrator provided by Anna Pollmann)
    G4double lBandWidth = 9.0 * mm;     //(taken from construction sketch of the Band w/o Penetrator provided by Anna Pollmann)

    // clamp dimensions
    G4double lClampLength = 10.0 * mm; // according to Fusion 360 model of the harness provided by Anna Pollmann

    // First band
    G4Tubs *lBandASolid = new G4Tubs("lBandASolid", mOM->mGlassOutRad, mOM->mGlassOutRad + lBandThickness, lBandWidth / 2, 0, CLHEP::pi);

    // Second band, this is the band with the ring for the penetrator
    G4Tubs *lBandBTempSolid = new G4Tubs("lBandBSolid_temp", mOM->mGlassOutRad - 1. * cm, mOM->mGlassOutRad + lBandThickness + 1. * cm, lBandWidth / 2 + 1. * cm, 90. * deg - mPlugAngle - 10. * deg, 20. * deg);
    G4SubtractionSolid *lBandBNoRingSolid = new G4SubtractionSolid("lBandBNoRingSolid", lBandASolid, lBandBTempSolid);

    // This band's ring.. the ring to rule them all!
    G4RotationMatrix lTempTubeRotation = G4RotationMatrix();
    lTempTubeRotation.rotateY(90. * deg);
    lTempTubeRotation.rotateX(90. * deg);
    lTempTubeRotation.rotateZ(90. * deg - mPlugAngle);
    G4EllipticalTube *lTempTubeOuterSolid = new G4EllipticalTube("lTempTubeOuterSolid", 35. * mm, 24.5 * mm, 2000. * mm);
    G4EllipticalTube *lTempTubeInnerSolid = new G4EllipticalTube("lTempTubeInnerSolid", 25. * mm, 17.49 * mm, 2500. * mm);
    G4SubtractionSolid *lTempTubeSolid = new G4SubtractionSolid("lTempTubeSolid", lTempTubeOuterSolid, lTempTubeInnerSolid);
    G4Sphere *lTempSphereSolid = new G4Sphere("lTempSphereSolid", mOM->mGlassOutRad, mOM->mGlassOutRad + lBandThickness, 0, CLHEP::pi, 0, CLHEP::pi);

    G4IntersectionSolid *lRingSolid = new G4IntersectionSolid("lRingSolid", lTempSphereSolid, lTempTubeSolid, G4Transform3D(lTempTubeRotation, G4ThreeVector(0, 0, 0)));
    // Now we union the ring with the band, which should be already aligned
    G4RotationMatrix lBandRotation = G4RotationMatrix();
    G4UnionSolid *lBandBSolid = new G4UnionSolid("lBandBSolid", lBandBNoRingSolid, lRingSolid, &lBandRotation, G4ThreeVector(0, 0, 0));

    // Separate bands into 2 different solids because of weird visualizacion problems
    lBandRotation = G4RotationMatrix();
    lBandRotation.rotateY(90 * deg);
    lBandRotation.rotateZ(180 * deg);
    lBandRotation.rotateX(180 * deg);

    G4UnionSolid *lBandsSolidTop = new G4UnionSolid("Band_solid_top", lBandASolid, lBandBSolid, &lBandRotation, G4ThreeVector(0, 0, 0));

    // bottom bands, this is twice lBandASolid
    G4UnionSolid *lBandsSolidBottom = new G4UnionSolid("Band_solid_bottom", lBandASolid, lBandASolid, &lBandRotation, G4ThreeVector(0, 0, 0));

    G4LogicalVolume *lBandsLogicalTop = new G4LogicalVolume(lBandsSolidTop, mData->getMaterial("NoOptic_Stahl"), "mBands logical top");
    G4LogicalVolume *lBandsLogicalBottom = new G4LogicalVolume(lBandsSolidBottom, mData->getMaterial("NoOptic_Stahl"), "mBands logical bottom");

    new G4LogicalSkinSurface("band_skin", lBandsLogicalTop, mData->getOpticalSurface("Surf_StainlessSteelGround"));
    new G4LogicalSkinSurface("band_skin", lBandsLogicalBottom, mData->getOpticalSurface("Surf_StainlessSteelGround"));

    lBandsLogicalTop->SetVisAttributes(mSteelVis);
    lBandsLogicalBottom->SetVisAttributes(mSteelVis);

    G4RotationMatrix lBandRotationTop = G4RotationMatrix();
    lBandRotationTop.rotateX(90 * deg);
    lBandRotationTop.rotateZ(-mHarnessRotAngle);
    G4ThreeVector lBandPosition = G4ThreeVector(0, 0, mOM->mCylHigh + 1. * mm);
    appendComponent(lBandsSolidTop, lBandsLogicalTop, lBandPosition, lBandRotationTop, "Band_Top");

    G4RotationMatrix lBandRotationBottom = G4RotationMatrix();
    lBandRotationBottom.rotateX(-90 * deg);
    lBandRotationBottom.rotateZ(-mHarnessRotAngle);
    appendComponent(lBandsSolidBottom, lBandsLogicalBottom, -lBandPosition, lBandRotationBottom, "Band_Bottom");
}

void mDOMHarness::bridgeRopesSolid()
{
    const G4double lBridgePhiStart = 83.0 * deg;
    const G4double lBridgeTotalPhi = 14.0 * deg;
    const G4double lBridgeRInner = mOM->mGlassOutRad + mTeraThickness + mPadThickness;
    const G4double lBridgeRInner_list[6] = {lBridgeRInner, lBridgeRInner, lBridgeRInner, lBridgeRInner, lBridgeRInner, lBridgeRInner};

    const G4double lBridgeROuterTemp[6] = {(lBridgeRInner + 2.2 * mm),
                                           (lBridgeRInner + 7.180628394561269 * mm),
                                           (lBridgeRInner + 12.255684206223851 * mm),
                                           (lBridgeRInner + 12.255684206223851 * mm),
                                           (lBridgeRInner + 7.180628394561269 * mm),
                                           (lBridgeRInner + 2.2 * mm)};

    const G4double lBridgeROuter[6] = {lBridgeROuterTemp[0],
                                       (lBridgeROuterTemp[0] + 10.83279768266868 * mm),
                                       mTotalWidth,
                                       mTotalWidth,
                                       (lBridgeROuterTemp[0] + 10.83279768266868 * mm),
                                       lBridgeROuterTemp[0]};

    const G4double lBridgeZPlane[6] = {-27.0 * mm,
                                       -16.16720231733132 * mm,
                                       -7.85 * mm,
                                       7.85 * mm,
                                       16.16720231733132 * mm,
                                       27.0 * mm};

    const G4double lBridgeZTemp[6] = {-17.0 * mm,
                                      -14.879793181282428 * mm,
                                      -7.85 * mm,
                                      7.85 * mm,
                                      14.879793181282428 * mm,
                                      17.0 * mm};

    G4Polycone *lBridgeTemp1Solid = new G4Polycone("BridgeSolid", lBridgePhiStart, lBridgeTotalPhi, 6, lBridgeZPlane, lBridgeRInner_list, lBridgeROuter);
    G4Polycone *lBridgeTemp2Solid = new G4Polycone("lBridgeTemp2Solid", lBridgePhiStart - 1.0 * deg, 16.0 * deg, 6, lBridgeZTemp, lBridgeRInner_list, lBridgeROuterTemp);
    G4SubtractionSolid *lBridgeSolid = new G4SubtractionSolid("", lBridgeTemp1Solid, lBridgeTemp2Solid);

    const G4double lRopeZShift = 2.0 * mm;

    G4Tubs *lRopeSolid = new G4Tubs("lRopeSolid", 0.0, mRopeRMax, mRopeDz - lRopeZShift, 0.0, 360.0 * deg);

    // First union
    G4RotationMatrix lRot = G4RotationMatrix();
    lRot.rotateX(mRopeRotationAngleX);

    G4ThreeVector lUnionPos = G4ThreeVector(0, mOM->mGlassOutRad - mRopeDz * sin(mRopeRotationAngleX) + 2.25 * cm, (mRopeDz)*cos(mRopeRotationAngleX));
    G4UnionSolid *lTempUnion1 = new G4UnionSolid("temp1", lBridgeSolid, lRopeSolid, G4Transform3D(lRot, lUnionPos));

    // second union, rope in the other side
    lRot = G4RotationMatrix();
    lRot.rotateX(-mRopeRotationAngleX);
    lUnionPos = G4ThreeVector(0, mOM->mGlassOutRad - mRopeDz * sin(mRopeRotationAngleX) + 2.25 * cm, -(mRopeDz)*cos(mRopeRotationAngleX));
    G4UnionSolid *lTempUnion2 = new G4UnionSolid("temp2", lTempUnion1, lRopeSolid, G4Transform3D(lRot, lUnionPos));

    // and now this same object 3 more times, with 90 degrees difference
    lRot = G4RotationMatrix();
    lRot.rotateZ(90 * deg);
    G4UnionSolid *lTempUnion3 = new G4UnionSolid("temp3", lTempUnion2, lTempUnion2, G4Transform3D(lRot, G4ThreeVector(0, 0, 0)));
    lRot.rotateZ(90 * deg);
    G4UnionSolid *lTempUnion4 = new G4UnionSolid("temp4", lTempUnion3, lTempUnion2, G4Transform3D(lRot, G4ThreeVector(0, 0, 0)));
    lRot.rotateZ(90 * deg);

    G4UnionSolid *lRopesUnionSolid = new G4UnionSolid("lRopesUnionSolid", lTempUnion4, lTempUnion2, G4Transform3D(lRot, G4ThreeVector(0, 0, 0)));
    G4LogicalVolume *lRopesUnionLogical = new G4LogicalVolume(lRopesUnionSolid, mData->getMaterial("NoOptic_Stahl"), "");

    new G4LogicalSkinSurface("ropes_skin", lRopesUnionLogical, mData->getOpticalSurface("Surf_StainlessSteelGround"));

    G4RotationMatrix lRopesRot = G4RotationMatrix();
    lRopesRot.rotateZ(mHarnessRotAngle);

    appendComponent(lRopesUnionSolid, lRopesUnionLogical, G4ThreeVector(0, 0, 0), lRopesRot, "Ropes");
}

void mDOMHarness::mainDataCable()
{
    const G4double lDataCableRadius = 26.0 * mm; // Radius of the main data cable (according to Prof. Kappes)
    const G4double lDataCableLength = 4.0 * m;   // Length of main data cable

    G4Tubs *lDataCableSolid = new G4Tubs("MainDataCable_solid", 0, lDataCableRadius, lDataCableLength / 2.0, 0, 2 * CLHEP::pi);

    G4LogicalVolume *lDataCableLogical = new G4LogicalVolume(lDataCableSolid, mData->getMaterial("NoOptic_Absorber"), "MainDataCable_logical");
    new G4LogicalSkinSurface("MainDataCable_skin", lDataCableLogical, mData->getOpticalSurface("Surf_BlackDuctTapePolished"));
    lDataCableLogical->SetVisAttributes(mAbsorberVis);

    G4double lMainCableAngle = 90.0 * deg;
    G4double lMainCableRadius = (mRopeStartingPoint + lDataCableRadius + mRopeRMax + 0.2 * cm - mBridgeAddedThickness - 0.75 * cm);
    G4ThreeVector lDataCablePosition = G4ThreeVector(lMainCableRadius * sin(lMainCableAngle),
                                                     lMainCableRadius * cos(lMainCableAngle),
                                                     0.0);

    appendComponent(lDataCableSolid, lDataCableLogical, lDataCablePosition, G4RotationMatrix(), "mainDataCable");
}

void mDOMHarness::pads()
{
    const G4double lPadWidth = 11.0 * mm;  // (taken from construction sketch of the rubber pads provided by Anna Pollmann)
    const G4double lPadAngle = 44.5 * deg; // (taken from construction sketch of the rubber pads provided by Anna Pollmann)

    G4Tubs *lPadSolid = new G4Tubs("lPadSolid", mOM->mGlassOutRad + mTeraThickness, mOM->mGlassOutRad + mTeraThickness + mPadThickness, lPadWidth / 2.0, lPadAngle / 2.0, lPadAngle);

    G4RotationMatrix lRot = G4RotationMatrix();

    G4UnionSolid *lTempPadUnion1 = new G4UnionSolid("temppad1", lPadSolid, lPadSolid, G4Transform3D(lRot, G4ThreeVector(0, 0, (-44.0) * mm)));
    lRot.rotateZ(90.0 * deg);
    G4UnionSolid *lTempPadUnion2 = new G4UnionSolid("temppad2", lTempPadUnion1, lTempPadUnion1, G4Transform3D(lRot, G4ThreeVector(0, 0, 0)));
    lRot.rotateZ(90.0 * deg);
    G4UnionSolid *lTempPadUnion3 = new G4UnionSolid("temppad3", lTempPadUnion2, lTempPadUnion1, G4Transform3D(lRot, G4ThreeVector(0, 0, 0)));
    lRot.rotateZ(90.0 * deg);

    G4UnionSolid *lPadsUnionSolid = new G4UnionSolid("lPadsUnionSolid", lTempPadUnion3, lTempPadUnion1, G4Transform3D(lRot, G4ThreeVector(0, 0, 0)));
    G4LogicalVolume *lPadsUnionLogical = new G4LogicalVolume(lPadsUnionSolid, mData->getMaterial("NoOptic_Absorber"), "");
    new G4LogicalSkinSurface("pad_skin", lPadsUnionLogical, mData->getOpticalSurface("Surf_PadSurface"));

    lPadsUnionLogical->SetVisAttributes(mRedVis);

    G4RotationMatrix lPadRot = G4RotationMatrix();
    lPadRot.rotateZ(mHarnessRotAngle - 14.5 * deg);
    appendComponent(lPadsUnionSolid, lPadsUnionLogical, G4ThreeVector(0, 0, (22.0) * mm), lPadRot, "pads");
}

void mDOMHarness::PCA()
{
    // Cylindrical metallic part
    const G4double lMushRadiusUpper = 10.965 * mm; // Radius of the (black) conical part of the penetrator at the thicker end (measured by hand)
    const G4double lMushRadiusLower = 15.0 * mm;   // Radius of the (black) conical part of the penetrator at the thinner end (according to construction sketch by Hydrogroup)
    const G4double lMushHeight = 32.77 * mm;       // Height of the penetrator part that is made up of a conical and a cylindrical part
    const G4double lMushCylHeight = 6.35 * mm;     // Height of cylindrical part (measured by hand)
    const G4double lMushConeHeight = lMushHeight - lMushCylHeight;

    G4Tubs *lMushCylSolid = new G4Tubs("Mush solid", 0, lMushRadiusLower, lMushCylHeight / 2., 0, 2. * CLHEP::pi);
    G4LogicalVolume *lMushCylLogical = new G4LogicalVolume(lMushCylSolid, mData->getMaterial("NoOptic_Stahl"), "Mush logical");
    new G4LogicalSkinSurface("mush_cyl_skin", lMushCylLogical, mData->getOpticalSurface("Surf_AluminiumGround"));
    lMushCylLogical->SetVisAttributes(mSteelVis);

    G4RotationMatrix lMushRot = G4RotationMatrix();
    lMushRot.rotateX(-mPlugAngle);
    lMushRot.rotateZ(mHarnessRotAngle - 90 * deg);
    G4ThreeVector lMushPosition = G4ThreeVector((mOM->mGlassOutRad + lMushCylHeight / 2) * (cos(mHarnessRotAngle)) * sin(mPlugAngle), (mOM->mGlassOutRad + lMushCylHeight / 2) * sin(mHarnessRotAngle) * sin(mPlugAngle), (mOM->mGlassOutRad + lMushCylHeight / 2) * cos(mPlugAngle) + mOM->mCylHigh);
    appendComponent(lMushCylSolid, lMushCylLogical, lMushPosition, lMushRot, "Mush");

    // Now building the hole penetrator cable as a single entity
    // first the conical cap
    G4Cons *lMushConeSolid = new G4Cons("mush_cone_solid", 0, lMushRadiusLower, 0, lMushRadiusUpper, lMushConeHeight / 2., 0, 2. * CLHEP::pi);

    // rigid part of the penetrator plastic, here it is called tube?
    const G4double lTubeRadius = 10.67 * mm; // Radius of the rigid part of the cable that exits the penetrator part (according to construction sketch by Hydrogroup)
    const G4double lTubeLength = 60.0 * mm;  // Length of the rigid part of the cable that exits the penetrator part (measured by hand)
    G4Tubs *lTubeSolid = new G4Tubs("tube_solid", 0, lTubeRadius, lTubeLength / 2, 0, 2 * CLHEP::pi);

    G4RotationMatrix lRot;
    lRot.rotateX(90 * deg);
    G4Transform3D lUnionTransform = G4Transform3D(lRot, G4ThreeVector(0, -lTubeLength / 2., lMushConeHeight / 2. - lTubeRadius));

    G4UnionSolid *lPenCableRigid_solid = new G4UnionSolid("Penetrator cable solid rigid part", lMushConeSolid, lTubeSolid, lUnionTransform);

    // Building PCA. This is gonna be built from outside to the inside, to do the unions easily
    // variables
    const G4double lWireThickness = 1.3716 * cm;                                                                                                                                                                                                                                                                                                                                                                                                                                                   // south bay cable width (0.54 inches) (according to construction sketch by South Bay Corporation)
    const G4double lWireRadius = 17.78 * cm;                                                                                                                                                                                                                                                                                                                                                                                                                                                       // minimum bending diameter of the cable (14 inches)
    const G4double lDistTubeToRope = ((mRopeStartingPoint) + mRopeRMax * (1 - cos(mRopeRotationAngleX)) - ((mOM->mGlassOutRad + lMushHeight - lTubeRadius) * cos(mPlugAngle) + lTubeLength * sin(mPlugAngle) + mOM->mCylHigh) * tan(mRopeRotationAngleX) - (mOM->mGlassOutRad + lMushHeight - lTubeRadius) * sin(mPlugAngle) + lTubeLength * cos(mPlugAngle)) / cos(mRopeRotationAngleX) - (lWireThickness / 2 + mRopeRMax) - mRopeDz * (1 - cos(mRopeRotationAngleX)) * sin(mRopeRotationAngleX); // Modified distance between start of first torus and end of second torus

    const G4double lPCA2angle = acos((0.5) * (lDistTubeToRope / lWireRadius + sin(mPlugAngle + mRopeRotationAngleX) + 1.));
    const G4double lPCA1angle = 90. * deg + lPCA2angle - (mPlugAngle + mRopeRotationAngleX);
    const G4double lExtnLength = 30. * 2.54 * cm - lWireRadius * (lPCA1angle + lPCA2angle) - lTubeLength;

    // PCA1
    G4Torus *lPCA1Solid = new G4Torus("lPCA1Solid", 0, lWireThickness / 2., lWireRadius, 0, lPCA1angle);
    // PCA2
    G4Torus *lPCA2Solid = new G4Torus("lPCA2Solid", 0, lWireThickness / 2., lWireRadius, 0, lPCA2angle);
    // Straight part of the cable
    G4Tubs *lExtnSolid = new G4Tubs("lExtnSolid", 0, lWireThickness / 2., lExtnLength / 2., 0, 2 * CLHEP::pi);

    // Now first union
    G4ThreeVector lPos1 = G4ThreeVector(lWireRadius, -lExtnLength / 2., 0);
    G4UnionSolid *lCableTempUnion1 = new G4UnionSolid("temp1", lPCA2Solid, lExtnSolid, G4Transform3D(lRot, lPos1));

    // Second union
    G4RotationMatrix lRot2 = G4RotationMatrix();
    lRot2.rotateZ(-lPCA2angle); // PCA2 ANGLE
    lRot2.rotateY(180 * deg);
    G4ThreeVector lPos2 = G4ThreeVector(2 * lWireRadius, 0, 0);
    G4UnionSolid *lCableTempUnion2 = new G4UnionSolid("temp2", lPCA1Solid, lCableTempUnion1, G4Transform3D(lRot2, lPos2));

    // Now we union the cable with the rigid part
    G4RotationMatrix lRot3 = G4RotationMatrix();
    lRot3.rotateY(90. * deg);
    lRot3.rotateX(-lPCA1angle);
    G4ThreeVector lPos3 = G4ThreeVector(0, -lTubeLength, lWireRadius + lMushConeHeight / 2. - lTubeRadius); // lMushConeHeight/2.-lTubeRadius is the translation of the previous union indeed

    G4UnionSolid *lPCASolid = new G4UnionSolid("PCACable_solid", lPenCableRigid_solid, lCableTempUnion2, G4Transform3D(lRot3, lPos3)); // reference frame of this object is the same as placing lMushConeSolid alone, so the part in contact with the metal part of the penetrator in the module
    G4LogicalVolume *lPCALogical = new G4LogicalVolume(lPCASolid, mData->getMaterial("NoOptic_Absorber"), "Penetrator cable logical");
    new G4LogicalSkinSurface("penetrator cable skin", lPCALogical, mData->getOpticalSurface("Surf_BlackDuctTapePolished"));
    lPCALogical->SetVisAttributes(mAbsorberVis);

    G4ThreeVector lPCAPosition = G4ThreeVector((mOM->mGlassOutRad + (2 * lMushCylHeight + lMushConeHeight) / 2.) * (cos(mHarnessRotAngle)) * sin(mPlugAngle), (mOM->mGlassOutRad + (2 * lMushCylHeight + lMushConeHeight) / 2) * sin(mHarnessRotAngle) * sin(mPlugAngle), (mOM->mGlassOutRad + (2 * lMushCylHeight + lMushConeHeight) / 2) * cos(mPlugAngle) + mOM->mCylHigh);
    appendComponent(lPCASolid, lPCALogical, lPCAPosition, lMushRot, "PCA");
}

void mDOMHarness::plug()
{
    const G4double lPlugRadius = 8.0 * mm;  // radius of the screw part of the penetrator (according to construction sketch by Hydrogroup)
    const G4double lPlugLength = 25.0 * mm; // length of the screw part of the penetrator (according to construction sketch by Hydrogroup)

    G4Tubs *lPlugSolid = new G4Tubs("plug_solid_temp", 0, lPlugRadius, lPlugLength / 2, 0, 2 * CLHEP::pi);

    G4ThreeVector lPlugPosition = G4ThreeVector((mOM->mGlassOutRad - lPlugLength / 2.) * cos(mHarnessRotAngle) * sin(mPlugAngle),
                                                (mOM->mGlassOutRad - lPlugLength / 2.) * sin(mHarnessRotAngle) * sin(mPlugAngle),
                                                mOM->mCylHigh + (mOM->mGlassOutRad - lPlugLength / 2) * cos(mPlugAngle));
    G4RotationMatrix lPlugRotation = G4RotationMatrix();
    lPlugRotation.rotateX(-mPlugAngle);
    lPlugRotation.rotateZ(mHarnessRotAngle - 90 * deg);

    G4LogicalVolume *lPlugLogical = new G4LogicalVolume(lPlugSolid, mData->getMaterial("NoOptic_Stahl"), "Plug_logical");
    new G4LogicalSkinSurface("plug_skin", lPlugLogical, mData->getOpticalSurface("Surf_AluminiumGround"));
    lPlugLogical->SetVisAttributes(mSteelVis);

    appendComponent(lPlugSolid, lPlugLogical, lPlugPosition, lPlugRotation, "Plug");
}

void mDOMHarness::teraBelt()
{
    const G4double lTeraWidth = 40.0 * mm; // replace with the actual value
    G4double zCorners[] = {lTeraWidth / 2, 0, -lTeraWidth / 2};
    G4double R = mOM->mGlassOutRad + 0.8 * mm; // add something so they do not touch
    G4double rCornersInn[] = {R, R + (lTeraWidth / 2) * sin(mOM->mCylinderAngle), R};
    R = mOM->mGlassOutRad + mTeraThickness;
    G4double rCornersOut[] = {R, R + (lTeraWidth / 2) * sin(mOM->mCylinderAngle), R};

    G4Polycone *lTeraSolidTemp1 = new G4Polycone("tera_solid_temp1", 0, 2 * CLHEP::pi, 3, rCornersInn, zCorners);
    G4Polycone *lTeraSolidTemp2 = new G4Polycone("tera_solid_temp2", 0, 2 * CLHEP::pi, 3, rCornersOut, zCorners);
    G4SubtractionSolid *lTeraSolid = new G4SubtractionSolid("tera_solid", lTeraSolidTemp2, lTeraSolidTemp1, G4Transform3D(G4RotationMatrix(), G4ThreeVector(0, 0, 0)));

    G4LogicalVolume *lTeraLogical = new G4LogicalVolume(lTeraSolid, mData->getMaterial("NoOptic_Absorber"), "tera_logical");
    new G4LogicalSkinSurface("tera_skin", lTeraLogical, mData->getOpticalSurface("Surf_BlackDuctTapePolished"));
    lTeraLogical->SetVisAttributes(mAbsorberVis);

    appendComponent(lTeraSolid, lTeraLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "TeraTapeBelt");
}
