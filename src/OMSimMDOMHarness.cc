/** @file OMSimMDOMHarness.cc
 *  @brief Construction of mDOM harness.
 *
 *  @author Cristian Lozano, Martin Unland
 *  @date November 2021
 *
 *  @version Geant4 10.7
 *
 */

#include "OMSimMDOMHarness.hh"
#include "OMSimMDOM.hh"
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


extern G4int gDOM;
extern G4bool gharness_ropes;
extern G4bool gVisual;
extern G4double gmdomseparation;
extern G4int gn_mDOMs;

mDOMHarness::mDOMHarness(mDOM* pMDOM, OMSimInputData* pData) {
    mOM = pMDOM;
    mData = pData;
    GetSharedData();
    Construction();
};

/**
 * Set all data members used in several functions
 */
void mDOMHarness::GetSharedData() {
    
    mPlugAngle = mData->GetValue(mDataKey, "jPlugAngle");      // this is the angle between the PMT rows, where the penetrator and the clamps are placed (according to Prof. Kappes) //TODO REDEFINED
    mPadThickness = mData->GetValue(mDataKey, "jPadThickness");
    mTeraThickness = mData->GetValue(mDataKey, "jTeraThickness");
    mRopeRMax = mData->GetValue(mDataKey, "jRopeRMax");
    mBridgeAddedThickness = mData->GetValue(mDataKey, "jBridgeAddedThickness");
    mRopeRotationAngleX = mData->GetValue(mDataKey, "jRopeRotationAngleX"); // this value is found by numerically solving the equation lBridgeROuter[2]+mRopeRMax/cos(mRopeRotationAngleX) = 2*mRopeDz*sin(mRopeRotationAngleX)- lBridgeZPlane[3]*tan(mRopeRotationAngleX) with wolframalpha.com LOL
    mRopeDz = mData->GetValue(mDataKey, "jRopeDz");
    mTotalWidth = mOM->mGlassOutRad + mTeraThickness + mPadThickness + mBridgeAddedThickness;
    lBridgeCorrection =  mData->GetValue(mDataKey, "jBridgeZPlane_2") * tan(mRopeRotationAngleX);  //  
    mRopeStartingPoint = mTotalWidth + lBridgeCorrection + mRopeRMax / cos(mRopeRotationAngleX); // this is the actual starting point of the rope, i.e. the distance to the z-axis, which has to be larger than lBridgeROuter[2] in order for the rope not to cut the bridge.
}

/**
 * The construction of each part is called
 */
void mDOMHarness::Construction() {
    
    BandsAndClamps();
    BridgeRopesSolid();
    MainDataCable();
    Pads();
    PCA();
    Plug();
    TeraBelt();
}

/**
 * Build bands with clamps and append it to component vector
 */
void mDOMHarness::BandsAndClamps()
{
    // vertical band dimensions
    G4double lBandThickness = mData->GetValue(mDataKey, "jBandThickness"); //(taken from construction sketch of the Band w/o Penetrator provided by Anna Pollmann)
    G4double lBandWidth = mData->GetValue(mDataKey, "jBandWidth");      //(taken from construction sketch of the Band w/o Penetrator provided by Anna Pollmann)

    // clamp dimensions
    G4double lClampLength = mData->GetValue(mDataKey, "jClampLength");  //according to Fusion 360 model of the harness provided by Anna Pollmann

    //First band
    G4Tubs* lBandASolid = new G4Tubs("lBandASolid", mOM->mGlassOutRad, mOM->mGlassOutRad + lBandThickness, lBandWidth / 2, 0, CLHEP::pi);

    //Second band, this is the band with the ring for the penetrator
    G4Tubs* lBandBTempSolid = new G4Tubs("lBandBSolid_temp", mOM->mGlassOutRad - 1. * cm, mOM->mGlassOutRad + lBandThickness + 1. * cm, lBandWidth / 2 + 1. * cm, 90. * deg - mPlugAngle - 10. * deg, 20. * deg);
    G4SubtractionSolid* lBandBNoRingSolid = new G4SubtractionSolid("lBandBNoRingSolid", lBandASolid, lBandBTempSolid);

    //This band's ring.. the ring to rule them all!
    G4RotationMatrix lTempTubeRotation = G4RotationMatrix();
    lTempTubeRotation.rotateY(90. * deg);
    lTempTubeRotation.rotateX(90. * deg);
    lTempTubeRotation.rotateZ(90. * deg - mPlugAngle);
    G4EllipticalTube* lTempTubeOuterSolid = new G4EllipticalTube("lTempTubeOuterSolid", 35. * mm, 24.5 * mm, 2000. * mm);
    G4EllipticalTube* lTempTubeInnerSolid = new G4EllipticalTube("lTempTubeInnerSolid", 25. * mm, 17.49 * mm, 2500. * mm);
    G4SubtractionSolid* lTempTubeSolid = new G4SubtractionSolid("lTempTubeSolid", lTempTubeOuterSolid, lTempTubeInnerSolid);
    G4Sphere* lTempSphereSolid = new G4Sphere("lTempSphereSolid", mOM->mGlassOutRad, mOM->mGlassOutRad + lBandThickness, 0, CLHEP::pi, 0, CLHEP::pi);

    G4IntersectionSolid* lRingSolid = new G4IntersectionSolid("lRingSolid", lTempSphereSolid, lTempTubeSolid, G4Transform3D(lTempTubeRotation, G4ThreeVector(0, 0, 0)));
    //Now we union the ring with the band, which should be already aligned
    G4RotationMatrix lBandRotation = G4RotationMatrix();
    G4UnionSolid* lBandBSolid = new G4UnionSolid("lBandBSolid", lBandBNoRingSolid, lRingSolid, &lBandRotation, G4ThreeVector(0, 0, 0));

    //Separate bands into 2 different solids because of weird visualizacion problems
    lBandRotation = G4RotationMatrix();
    lBandRotation.rotateY(90 * deg);
    lBandRotation.rotateZ(180 * deg);
    lBandRotation.rotateX(180 * deg);

    G4UnionSolid* lBandsSolidTop = new G4UnionSolid("Band_solid_top", lBandASolid, lBandBSolid, &lBandRotation, G4ThreeVector(0, 0, 0));

    //bottom bands, this is twice lBandASolid
    G4UnionSolid* lBandsSolidBottom = new G4UnionSolid("Band_solid_bottom", lBandASolid, lBandASolid, &lBandRotation, G4ThreeVector(0, 0, 0));

    G4LogicalVolume* lBandsLogicalTop = new G4LogicalVolume(lBandsSolidTop, mData->GetMaterial("NoOptic_Stahl"), "mBands logical top");
    G4LogicalVolume* lBandsLogicalBottom = new G4LogicalVolume(lBandsSolidBottom, mData->GetMaterial("NoOptic_Stahl"), "mBands logical bottom");

    new G4LogicalSkinSurface("band_skin", lBandsLogicalTop, mData->GetOpticalSurface("Refl_StainlessSteelGround"));
    new G4LogicalSkinSurface("band_skin", lBandsLogicalBottom, mData->GetOpticalSurface("Refl_StainlessSteelGround"));

    lBandsLogicalTop->SetVisAttributes(mSteelVis);
    lBandsLogicalBottom->SetVisAttributes(mSteelVis);

    G4RotationMatrix lBandRotationTop = G4RotationMatrix();
    lBandRotationTop.rotateX(90 * deg);
    lBandRotationTop.rotateZ(-mHarnessRotAngle);
    G4ThreeVector lBandPosition = G4ThreeVector(0, 0, mOM->mCylHigh + 1. * mm);
    AppendComponent(lBandsSolidTop, lBandsLogicalTop, lBandPosition, lBandRotationTop, "Band_Top");

    G4RotationMatrix lBandRotationBottom = G4RotationMatrix();
    lBandRotationBottom.rotateX(-90 * deg);
    lBandRotationBottom.rotateZ(-mHarnessRotAngle);
    AppendComponent(lBandsSolidBottom, lBandsLogicalBottom, -lBandPosition, lBandRotationBottom, "Band_Bottom");
}



/**
 * Build bridges & ropes and append it to component vector
 */
void mDOMHarness::BridgeRopesSolid()
{
    const G4double lBridgePhiStart = mData->GetValue(mDataKey, "jBridgePhiStart");
    const G4double lBridgeTotalPhi = mData->GetValue(mDataKey, "jBridgeTotalPhi");

    const G4double lBridgeRInner = mOM->mGlassOutRad + mTeraThickness + mPadThickness;
    const G4double lBridgeRInner_list[6] = { lBridgeRInner, lBridgeRInner, lBridgeRInner, lBridgeRInner, lBridgeRInner, lBridgeRInner };

    const G4double lBridgeROuterTemp[6] = { (lBridgeRInner + mData->GetValue(mDataKey, "jBridgeROutTemp_0")), // BridgeROutTemp_0 = 2.2mm
                                      (lBridgeRInner + mData->GetValue(mDataKey, "jBridgeROutTemp_1")), // BridgeROutTemp_1 = 2.2mm + 5.5 cos( 25.1 deg)
                                      (lBridgeRInner + mData->GetValue(mDataKey, "jBridgeROutTemp_2")), // BridgeROutTemp_2 = 2.2mm + 5.5 cos( 25.1 deg) + 7.4 * cos(46.7 deg)
                                      (lBridgeRInner + mData->GetValue(mDataKey, "jBridgeROutTemp_2")),
                                      (lBridgeRInner + mData->GetValue(mDataKey, "jBridgeROutTemp_1")),
                                      (lBridgeRInner + mData->GetValue(mDataKey, "jBridgeROutTemp_0")) };

    const G4double lBridgeROuter[6] = { lBridgeROuterTemp[0],
                                      (lBridgeROuterTemp[0] + mData->GetValue(mDataKey, "jBridgeZPlaneTrig_1")), // BridgeZPlaneTrig_1 = sin(50.2 deg) * 14.1
                                      mTotalWidth,
                                      mTotalWidth,
                                      (lBridgeROuterTemp[0] + mData->GetValue(mDataKey, "jBridgeZPlaneTrig_1")),
                                      lBridgeROuterTemp[0] };

    const G4double lBridgeZPlane[6] = { -mData->GetValue(mDataKey, "jBridgeZPlane_0"), // BridgeZPlane_0 = 27mm
                                      -mData->GetValue(mDataKey, "jBridgeZPlane_1"), // BridgeZPlane_1 = 27 - sin(50.2 deg) * 14.1
                                      -mData->GetValue(mDataKey, "jBridgeZPlane_2"), // BridgeZPlane_2 = 7.85
                                      mData->GetValue(mDataKey, "jBridgeZPlane_2"),
                                      mData->GetValue(mDataKey, "jBridgeZPlane_1"),
                                      mData->GetValue(mDataKey, "jBridgeZPlane_0") };

    const G4double lBridgeZTemp[6] = { -mData->GetValue(mDataKey, "jBridgeZTemp_0"), // BridgeZPlane_0 = 17 mm
                                      -mData->GetValue(mDataKey, "jBridgeZTemp_1"), // BridgeZPlane_1 = 7.85 + sin(71.8 deg) * 7.4
                                      -mData->GetValue(mDataKey, "jBridgeZTemp_2"), // BridgeZPlane_2 = 7.85
                                      mData->GetValue(mDataKey, "jBridgeZTemp_2"),
                                      mData->GetValue(mDataKey, "jBridgeZTemp_1"),
                                      mData->GetValue(mDataKey, "jBridgeZTemp_0") };



    G4Polycone* lBridgeTemp1Solid = new G4Polycone("BridgeSolid", lBridgePhiStart, lBridgeTotalPhi, 6, lBridgeZPlane, lBridgeRInner_list, lBridgeROuter);
    G4Polycone* lBridgeTemp2Solid = new G4Polycone("lBridgeTemp2Solid", lBridgePhiStart - 1. * deg, 16. * deg, 6, lBridgeZTemp, lBridgeRInner_list, lBridgeROuterTemp);
    G4SubtractionSolid* lBridgeSolid = new G4SubtractionSolid("", lBridgeTemp1Solid, lBridgeTemp2Solid);

    const G4double lRopeZShift = mData->GetValue(mDataKey, "jRopeZShift"); //dont know why.. without it the vis breaks

    G4Tubs* lRopeSolid = new G4Tubs("lRopeSolid", 0, mRopeRMax, mRopeDz - lRopeZShift, 0, 360. * deg);


    //First union
    G4RotationMatrix lRot = G4RotationMatrix();
    lRot.rotateX(mRopeRotationAngleX);

    G4ThreeVector lUnionPos = G4ThreeVector(0, mOM->mGlassOutRad - mRopeDz * sin(mRopeRotationAngleX) + 2.25 * cm, (mRopeDz)*cos(mRopeRotationAngleX));
    G4UnionSolid* lTempUnion1 = new G4UnionSolid("temp1", lBridgeSolid, lRopeSolid, G4Transform3D(lRot, lUnionPos));

    //second union, rope in the other side
    lRot = G4RotationMatrix();
    lRot.rotateX(-mRopeRotationAngleX);
    lUnionPos = G4ThreeVector(0, mOM->mGlassOutRad - mRopeDz * sin(mRopeRotationAngleX) + 2.25 * cm, -(mRopeDz)*cos(mRopeRotationAngleX));
    G4UnionSolid* lTempUnion2 = new G4UnionSolid("temp2", lTempUnion1, lRopeSolid, G4Transform3D(lRot, lUnionPos));

    //and now this same object 3 more times, with 90 degrees difference
    lRot = G4RotationMatrix();
    lRot.rotateZ(90 * deg);
    G4UnionSolid* lTempUnion3 = new G4UnionSolid("temp3", lTempUnion2, lTempUnion2, G4Transform3D(lRot, G4ThreeVector(0, 0, 0)));
    lRot.rotateZ(90 * deg);
    G4UnionSolid* lTempUnion4 = new G4UnionSolid("temp4", lTempUnion3, lTempUnion2, G4Transform3D(lRot, G4ThreeVector(0, 0, 0)));
    lRot.rotateZ(90 * deg);

    G4UnionSolid* lRopesUnionSolid = new G4UnionSolid("lRopesUnionSolid", lTempUnion4, lTempUnion2, G4Transform3D(lRot, G4ThreeVector(0, 0, 0)));
    G4LogicalVolume* lRopesUnionLogical = new G4LogicalVolume(lRopesUnionSolid, mData->GetMaterial("NoOptic_Stahl"), "");

    new G4LogicalSkinSurface("ropes_skin", lRopesUnionLogical, mData->GetOpticalSurface("Refl_StainlessSteelGround"));

    G4RotationMatrix lRopesRot = G4RotationMatrix();
    lRopesRot.rotateZ(mHarnessRotAngle);

    AppendComponent(lRopesUnionSolid, lRopesUnionLogical, G4ThreeVector(0, 0, 0), lRopesRot, "Ropes");
}

/**
 * Build main data cable and append it to component vector
 */
void mDOMHarness::MainDataCable()
{
    const G4double lDataCableRadius = mData->GetValue(mDataKey, "jDataCableRadius"); // Radius of the main data cable (according to Prof. Kappes)
    const G4double lDataCableLength = mData->GetValue(mDataKey, "jDataCableLength"); // Length of main data cable

    G4Tubs* lDataCableSolid = new G4Tubs("MainDataCable_solid", 0, lDataCableRadius, lDataCableLength / 2., 0, 2 * CLHEP::pi);

    G4LogicalVolume* lDataCableLogical = new G4LogicalVolume(lDataCableSolid, mData->GetMaterial("NoOptic_Absorber"), "MainDataCable_logical");
    new G4LogicalSkinSurface("MainDataCable_skin", lDataCableLogical, mData->GetOpticalSurface("Refl_BlackDuctTapePolished"));
    lDataCableLogical->SetVisAttributes(mAbsorberVis);

    G4ThreeVector lDataCablePosition = G4ThreeVector((mRopeStartingPoint + lDataCableRadius + mRopeRMax + 0.5 * cm) * sin(mHarnessRotAngle),
        (mRopeStartingPoint + lDataCableRadius + mRopeRMax + 0.5 * cm) * cos(mHarnessRotAngle),
        0);

    AppendComponent(lDataCableSolid, lDataCableLogical, lDataCablePosition, G4RotationMatrix(), "MainDataCable");
}


/**
 * Build pads and append it to component vector
 */
void mDOMHarness::Pads()

{
    const G4double lPadWidth = mData->GetValue(mDataKey, "jPadWidth");  // (taken from construction sketch of the rubber pads provided by Anna Pollmann)
    const G4double lPadAngle = mData->GetValue(mDataKey, "jPadAngle") / (2 * CLHEP::pi * mOM->mGlassOutRad) * 360. * deg; // (taken from construction sketch of the rubber pads provided by Anna Pollmann)
    G4Tubs* lPadSolid = new G4Tubs("lPadSolid", mOM->mGlassOutRad + mTeraThickness, mOM->mGlassOutRad + mTeraThickness + mPadThickness, lPadWidth / 2, lPadAngle / 2, lPadAngle);

    G4RotationMatrix lRot = G4RotationMatrix();

    G4UnionSolid* lTempPadUnion1 = new G4UnionSolid("temppad1", lPadSolid, lPadSolid, G4Transform3D(lRot, G4ThreeVector(0, 0, (-44.) * mm)));
    lRot.rotateZ(90 * deg);
    G4UnionSolid* lTempPadUnion2 = new G4UnionSolid("temppad2", lTempPadUnion1, lTempPadUnion1, G4Transform3D(lRot, G4ThreeVector(0, 0, 0)));
    lRot.rotateZ(90 * deg);
    G4UnionSolid* lTempPadUnion3 = new G4UnionSolid("temppad3", lTempPadUnion2, lTempPadUnion1, G4Transform3D(lRot, G4ThreeVector(0, 0, 0)));
    lRot.rotateZ(90 * deg);

    G4UnionSolid* lPadsUnionSolid = new G4UnionSolid("lPadsUnionSolid", lTempPadUnion3, lTempPadUnion1, G4Transform3D(lRot, G4ThreeVector(0, 0, 0)));
    G4LogicalVolume* lPadsUnionLogical = new G4LogicalVolume(lPadsUnionSolid, mData->GetMaterial("NoOptic_Absorber"), "");
    new G4LogicalSkinSurface("pad_skin", lPadsUnionLogical, mData->GetOpticalSurface("Refl_PadSurface"));

    lPadsUnionLogical->SetVisAttributes(mRedVis);

    G4RotationMatrix lPadRot = G4RotationMatrix();
    lPadRot.rotateZ(mHarnessRotAngle - 14.5 * deg);
    AppendComponent(lPadsUnionSolid, lPadsUnionLogical, G4ThreeVector(0, 0, (22) * mm), lPadRot, "Pads");

}

/**
 * Build PCA cable and append it to component vector
 */
void mDOMHarness::PCA()
{
    //Cylindrical metallic part
    const G4double lMushRadiusUpper = mData->GetValue(mDataKey, "jMushRadiusUpper"); //Radius of the (black) conical part of the penetrator at the thicker end (measured by hand)
    const G4double lMushRadiusLower = mData->GetValue(mDataKey, "jMushRadiusLower"); //Radius of the (black) conical part of the penetrator at the thinner end (according to construction sketch by Hydrogroup)
    const G4double lMushHeight = mData->GetValue(mDataKey, "jMushHeight");           //Height of the penetrator part that is made up of a conical and a cylindrical part
    const G4double lMushCylHeight = mData->GetValue(mDataKey, "jMushCylHeight");     //Height of cylindrical part (measured by hand)
    const G4double lMushConeHeight = lMushHeight - lMushCylHeight;

    G4Tubs* lMushCylSolid = new G4Tubs("Mush solid", 0, lMushRadiusLower, lMushCylHeight / 2., 0, 2. * CLHEP::pi);
    G4LogicalVolume* lMushCylLogical = new G4LogicalVolume(lMushCylSolid, mData->GetMaterial("NoOptic_Stahl"), "Mush logical");
    new G4LogicalSkinSurface("mush_cyl_skin", lMushCylLogical, mData->GetOpticalSurface("Refl_AluminiumGround"));
    lMushCylLogical->SetVisAttributes(mSteelVis);


    G4RotationMatrix lMushRot = G4RotationMatrix();
    lMushRot.rotateX(-mPlugAngle);
    lMushRot.rotateZ(mHarnessRotAngle - 90 * deg);
    G4ThreeVector lMushPosition = G4ThreeVector((mOM->mGlassOutRad + lMushCylHeight / 2) * (cos(mHarnessRotAngle)) * sin(mPlugAngle), (mOM->mGlassOutRad + lMushCylHeight / 2) * sin(mHarnessRotAngle) * sin(mPlugAngle), (mOM->mGlassOutRad + lMushCylHeight / 2) * cos(mPlugAngle) + mOM->mCylHigh);

    AppendComponent(lMushCylSolid, lMushCylLogical, lMushPosition, lMushRot, "Mush");


    //Now bulding the hole penetrator cable as a single entity
    //first the conical cap
    G4Cons* lMushConeSolid = new G4Cons("mush_cone_solid", 0, lMushRadiusLower, 0, lMushRadiusUpper, lMushConeHeight / 2., 0, 2. * CLHEP::pi);

    //rigid part of the penetrator plastic, here it is called tube?
    const G4double lTubeRadius = mData->GetValue(mDataKey, "jTubeRadius"); //Radius of the rigid part of the cable that exits the penetrator part (according to construction sketch by Hydrogroup)
    const G4double lTubeLength = mData->GetValue(mDataKey, "jTubeLength"); //Length of the rigid part of the cable that exits the penetrator part (measured by hand)
    G4Tubs* lTubeSolid = new G4Tubs("tube_solid", 0, lTubeRadius, lTubeLength / 2, 0, 2 * CLHEP::pi);

    G4RotationMatrix lRot;
    lRot.rotateX(90 * deg);
    G4Transform3D lUnionTransform = G4Transform3D(lRot, G4ThreeVector(0, -lTubeLength / 2., lMushConeHeight / 2. - lTubeRadius));

    G4UnionSolid* lPenCableRigid_solid = new G4UnionSolid("Penetrator cable solid rigid part", lMushConeSolid, lTubeSolid, lUnionTransform);

    //Bulding PCA. This is gonna be build from outside to the inside, to do the unions easily
    //variables
    const G4double lWireThickness = mData->GetValue(mDataKey, "jWireThickness"); // south bay cable width (0.54 inches) (according to construction sketch by South Bay Corporation)
    const G4double lWireRadius = mData->GetValue(mDataKey, "jWireRadius");        // minimum bending diameter of the cable (14 inches)
    const G4double lDistTubeToRope = ((mRopeStartingPoint)+mRopeRMax * (1 - cos(mRopeRotationAngleX)) - ((mOM->mGlassOutRad + lMushHeight - lTubeRadius) * cos(mPlugAngle) + lTubeLength * sin(mPlugAngle) + mOM->mCylHigh) * tan(mRopeRotationAngleX) - (mOM->mGlassOutRad + lMushHeight - lTubeRadius) * sin(mPlugAngle) + lTubeLength * cos(mPlugAngle)) / cos(mRopeRotationAngleX) - (lWireThickness / 2 + mRopeRMax) - mRopeDz * (1 - cos(mRopeRotationAngleX)) * sin(mRopeRotationAngleX); // Modified distance between start of first torus and end of second torus

    const G4double lPCA2angle = acos((0.5) * (-lDistTubeToRope / lWireRadius + sin(mPlugAngle + mRopeRotationAngleX) + 1.));
    const G4double lPCA1angle = 90. * deg + lPCA2angle - (mPlugAngle + mRopeRotationAngleX);
    const G4double lExtnLength = 30. * 2.54 * cm - lWireRadius * (lPCA1angle + lPCA2angle) - lTubeLength;
    //PCA1
    G4Torus* lPCA1Solid = new G4Torus("lPCA1Solid", 0, lWireThickness / 2., lWireRadius, 0, lPCA1angle);
    //PCA2
    G4Torus* lPCA2Solid = new G4Torus("lPCA2Solid", 0, lWireThickness / 2., lWireRadius, 0, lPCA2angle);
    //Straight part of the cable
    G4Tubs* lExtnSolid = new G4Tubs("lExtnSolid", 0, lWireThickness / 2., lExtnLength / 2., 0, 2 * CLHEP::pi);

    //Now first union
    G4ThreeVector lPos1 = G4ThreeVector(lWireRadius, -lExtnLength / 2., 0);
    G4UnionSolid* lCableTempUnion1 = new G4UnionSolid("temp1", lPCA2Solid, lExtnSolid, G4Transform3D(lRot, lPos1));

    //Second union
    G4RotationMatrix lRot2 = G4RotationMatrix();
    lRot2.rotateZ(-lPCA2angle); //PCA2 ANGLE
    lRot2.rotateY(180 * deg);

    G4ThreeVector lPos2 = G4ThreeVector(2 * lWireRadius, 0, 0);
    G4UnionSolid* lCableTempUnion2 = new G4UnionSolid("temp2", lPCA1Solid, lCableTempUnion1, G4Transform3D(lRot2, lPos2));

    //Now we union the cable with the rigid part
    G4RotationMatrix lRot3 = G4RotationMatrix();
    lRot3.rotateY(90. * deg);
    lRot3.rotateX(-lPCA1angle);
    G4ThreeVector lPos3 = G4ThreeVector(0, -lTubeLength, lWireRadius + lMushConeHeight / 2. - lTubeRadius); // lMushConeHeight/2.-lTubeRadius is the translation of the previous union indeed

    G4UnionSolid* lPCASolid = new G4UnionSolid("PCACable_solid", lPenCableRigid_solid, lCableTempUnion2, G4Transform3D(lRot3, lPos3)); //reference frame of this object is the same as placing lMushConeSolid alone, so the part in contact with the metal part of the penetrator in the module
    G4LogicalVolume* lPCALogical = new G4LogicalVolume(lPCASolid, mData->GetMaterial("NoOptic_Absorber"), "Penetrator cable logical");
    new G4LogicalSkinSurface("penetrator cable skin", lPCALogical, mData->GetOpticalSurface("Refl_BlackDuctTapePolished"));
    lPCALogical->SetVisAttributes(mAbsorberVis);


    G4ThreeVector lPCAPosition = G4ThreeVector((mOM->mGlassOutRad + (2 * lMushCylHeight + lMushConeHeight) / 2.) * (cos(mHarnessRotAngle)) * sin(mPlugAngle), (mOM->mGlassOutRad + (2 * lMushCylHeight + lMushConeHeight) / 2) * sin(mHarnessRotAngle) * sin(mPlugAngle), (mOM->mGlassOutRad + (2 * lMushCylHeight + lMushConeHeight) / 2) * cos(mPlugAngle) + mOM->mCylHigh);
    AppendComponent(lPCASolid, lPCALogical, lPCAPosition, lMushRot, "PCA");
}


/**
 * Build PCA plug and append it to component vector
 */
void mDOMHarness::Plug()
{
    const G4double lPlugRadius = mData->GetValue(mDataKey, "jPlugRadius");    //radius of the screw part of the penetrator (according to construction sketch by Hydrogroup)
    const G4double lPlugLength = mData->GetValue(mDataKey, "jPlugLength");    //length of the screw part of the penetrator (according to construction sketch by Hydrogroup)

    G4Tubs* lPlugSolid = new G4Tubs("plug_solid_temp", 0, lPlugRadius, lPlugLength / 2, 0, 2 * CLHEP::pi);

    G4ThreeVector lPlugPosition = G4ThreeVector((mOM->mGlassOutRad - lPlugLength / 2.) * cos(mHarnessRotAngle) * sin(mPlugAngle),
        (mOM->mGlassOutRad - lPlugLength / 2.) * sin(mHarnessRotAngle) * sin(mPlugAngle),
        mOM->mCylHigh + (mOM->mGlassOutRad - lPlugLength / 2) * cos(mPlugAngle));
    G4RotationMatrix lPlugRotation = G4RotationMatrix();
    lPlugRotation.rotateX(-mPlugAngle);
    lPlugRotation.rotateZ(mHarnessRotAngle - 90 * deg);

    G4LogicalVolume* lPlugLogical = new G4LogicalVolume(lPlugSolid, mData->GetMaterial("NoOptic_Stahl"), "Plug_logical");
    new G4LogicalSkinSurface("plug_skin", lPlugLogical, mData->GetOpticalSurface("Refl_AluminiumGround"));
    lPlugLogical->SetVisAttributes(mSteelVis);

    AppendComponent(lPlugSolid, lPlugLogical, lPlugPosition, lPlugRotation, "Plug");

}


/**
 * Build tera tape belt and append it to component vector
 */
void mDOMHarness::TeraBelt()
{
    const G4double lTeraWidth = mData->GetValue(mDataKey, "jTeraWidth") ;
    G4double zCorners[] = {  lTeraWidth/ 2, 0, -lTeraWidth/ 2};
    G4double R = mOM->mGlassOutRad+0.8*mm; //add something so they do not touch
    G4double rCornersInn[] = {  R, R + (lTeraWidth / 2) * sin(mOM->mCylinderAngle), R };
    R = mOM->mGlassOutRad + mTeraThickness;
    G4double rCornersOut[] = {  R, R + (lTeraWidth / 2) * sin(mOM->mCylinderAngle), R};

    G4Polycone* lTeraSolidTemp1 = new G4Polycone("tera_solid_temp1", 0, 2 * CLHEP::pi, 3, rCornersInn, zCorners);
    G4Polycone* lTeraSolidTemp2 = new G4Polycone("tera_solid_temp2", 0, 2 * CLHEP::pi, 3, rCornersOut, zCorners);
    G4SubtractionSolid* lTeraSolid = new G4SubtractionSolid("tera_solid", lTeraSolidTemp2, lTeraSolidTemp1, G4Transform3D(G4RotationMatrix(), G4ThreeVector(0,0,0)) );
    
    G4LogicalVolume* lTeraLogical = new G4LogicalVolume(lTeraSolid, mData->GetMaterial("NoOptic_Absorber"), "tera_logical");
    new G4LogicalSkinSurface("tera_skin", lTeraLogical, mData->GetOpticalSurface("Refl_BlackDuctTapePolished"));
    lTeraLogical->SetVisAttributes(mAbsorberVis);

    AppendComponent(lTeraSolid, lTeraLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "TeraTapeBelt");
}



