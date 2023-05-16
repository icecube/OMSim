/** @file OMSimMDOM.cc
 *  @brief Construction of mDOM.
 *
 *  @author Lew Classen, Martin Unland
 *  @date November 2021
 *
 *  @version Geant4 10.7
 */

#include "OMSimMDOM.hh"
#include "OMSimMDOMHarness.hh"
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
#include "OMSimCommandArgsTable.hh"

mDOM::mDOM(OMSimInputData* pData, G4bool pPlaceHarness) {
    mPlaceHarness = false;//pPlaceHarness;
    mData = pData;
    mPMTManager = new OMSimPMTConstruction(mData);
    if (OMSimCommandArgsTable::getInstance().get<bool>("visual")){
    mPMTManager->SelectPMT("pmt_Hamamatsu_R15458_1mm");
    }
    else{
       mPMTManager->SelectPMT("pmt_Hamamatsu_R15458_20nm"); 
       mPMTManager->SimulateInternalReflections();
    }
    mPMTManager->Construction();
    GetSharedData();
    if (mPlaceHarness){
         mHarness = new mDOMHarness(this, mData);
         IntegrateDetectorComponent(mHarness, G4ThreeVector(0,0,0), G4RotationMatrix(), "");
    }
    Construction();
}

void mDOM::GetSharedData() {
    mGlassOutRad = mData->GetValue(mDataKey, "jGlassOutRad"); // outer radius of galss cylinder (pressure vessel)
    mCylHigh = mData->GetValue(mDataKey, "jCylHigh");         // height of cylindrical part of glass half-vessel
    mGlassThick = mData->GetValue(mDataKey, "jGlassThick");                     // maximum Glass thickness
    mGelThicknessFrontPMT = mData->GetValue(mDataKey, "jGelThicknessFrontPMT"); // distance between inner glass surface and tip of PMTs
    mGelThickness = mData->GetValue(mDataKey, "jGelThickness");                 // distance between inner glass surface and holding structure, filled with gel
    mEqPMTrOffset = mData->GetValue(mDataKey, "jEqPMTrOffset"); // middle PMT circles are slightly further out due to mEqPMTzOffset
    mEqPMTzOffset = mData->GetValue(mDataKey, "jEqPMTzOffset"); // z-offset of middle PMT circles w.r.t. center of glass sphere
    mRefConeHalfZ = mData->GetValue(mDataKey, "jRefConeHalfZ"); // half-height of reflector (before cutting to right form)
    mRefConeSheetThickness = mData->GetValue(mDataKey, "mRefConeSheetThickness"); // aluminum sheet thickness true for all reflective cones
    mThetaPolar = mData->GetValue(mDataKey, "jThetaPolar");
    mThetaEquatorial = mData->GetValue(mDataKey, "jThetaEquatorial");
    mCylinderAngle = mData->GetValue(mDataKey, "jCylinderAngle");  // Deviation angle of cylindrical part of the pressure vessel
    mNrPolarPMTs = mData->GetValue(mDataKey,"jNrPolarPMTs");
    mNrEqPMTs = mData->GetValue(mDataKey,"jNrEqPMTs");
    mGlassInRad = mGlassOutRad - mGlassThick;
    mRefConeAngle = mData->GetValue(mDataKey, "jReflectorConeAngle");
    mTotalNrPMTs = (mNrPolarPMTs + mNrEqPMTs) * 2;
    mPMToffset = mPMTManager->GetDistancePMTCenterToPMTtip();
    mRefConeIdealInRad = mPMTManager->GetMaxPMTMaxRadius() + 2 * mm;
    mSupStructureRad = mGlassOutRad - mGlassThick - mGelThickness;
}


void mDOM::Construction()
{   mComponents.clear();
    SetPMTPositions();
    SetLEDPositions();

    G4UnionSolid* lGlassSolid = PressureVessel(mGlassOutRad, "Glass");
    G4UnionSolid* lGelSolid = PressureVessel(mGlassInRad, "Gel");

    G4SubtractionSolid* lSupStructureSolid;
    G4UnionSolid* lSupStructureFirstSolid;

    std::tie(lSupStructureSolid, lSupStructureFirstSolid) = SupportStructure();

    //Reflector solid
    G4Cons* lRefConeBasicSolid = new G4Cons("RefConeBasic", mRefConeIdealInRad,
        mRefConeIdealInRad + mRefConeSheetThickness / cos(mRefConeAngle),
        mRefConeIdealInRad + 2 * mRefConeHalfZ * tan(mRefConeAngle), mRefConeIdealInRad + mRefConeSheetThickness / cos(mRefConeAngle) + 2 * mRefConeHalfZ * tan(mRefConeAngle), mRefConeHalfZ, 0, 2 * CLHEP::pi);

    //Reflector cone for polar PMTs
    G4Ellipsoid* lSupStructureTopSolid = new G4Ellipsoid("FoamSphereTop solid", mSupStructureRad, mSupStructureRad, mSupStructureRad, -5 * mm, mSupStructureRad + 5 * mm);
    G4IntersectionSolid* lRefConePolarSolid = new G4IntersectionSolid("PolarRefCones", lRefConeBasicSolid, lSupStructureTopSolid, 0, G4ThreeVector(0, 0, -(mGlassInRad - mGelThicknessFrontPMT - mPMToffset + mRefConeHalfZ)));

    //Reflector cone for equatorial PMTs
    G4SubtractionSolid* lRefConeEqUpCutSolid = EquatorialReflector(lSupStructureFirstSolid, lRefConeBasicSolid, mThetaEquatorial, "upper");
    G4SubtractionSolid* lRefConeEqLoCutSolid = EquatorialReflector(lSupStructureFirstSolid, lRefConeBasicSolid, 180. * deg - mThetaEquatorial, "lower");

    //LED flashers
    G4UnionSolid* lLEDHoleAirSolid;
    G4UnionSolid* lLEDSolid;
    G4Tubs* lLEDGlassTopSolid;

    std::tie(lSupStructureSolid, lLEDSolid, lLEDHoleAirSolid, lLEDGlassTopSolid) = LedFlashers(lSupStructureSolid);

    //Logicals
    G4LogicalVolume* lGelLogical = new G4LogicalVolume(lGelSolid,
        mData->GetMaterial("RiAbs_Gel_Shin-Etsu"),
        "Gelcorpus logical");
    G4LogicalVolume* lSupStructureLogical = new G4LogicalVolume(lSupStructureSolid,
        mData->GetMaterial("NoOptic_Absorber"),
        "TubeHolder logical");
    G4LogicalVolume* lGlassLogical = new G4LogicalVolume(lGlassSolid,
        mData->GetMaterial("RiAbs_Glass_Vitrovex"),
        "Glass_log");
    if (mPlaceHarness)
    {
        lGelLogical = new G4LogicalVolume(SubstractHarnessPlug(lGelSolid),
            mData->GetMaterial("RiAbs_Gel_Shin-Etsu"),
            "Gelcorpus logical");
        lSupStructureLogical = new G4LogicalVolume(SubstractHarnessPlug(lSupStructureSolid),
            mData->GetMaterial("NoOptic_Absorber"),
            "TubeHolder logical");
        lGlassLogical = new G4LogicalVolume(SubstractHarnessPlug(lGlassSolid),
            mData->GetMaterial("RiAbs_Glass_Vitrovex"),
            "Glass_log");
    }
    G4LogicalVolume* lRefConePolarLogical = new G4LogicalVolume(lRefConePolarSolid,
        mData->GetMaterial("NoOptic_Reflector"),
        "RefConeType1 logical");

    G4LogicalVolume* lRefconeEqUpCutLogical = new G4LogicalVolume(lRefConeEqUpCutSolid,
        mData->GetMaterial("NoOptic_Reflector"),
        "RefConeType2 ETEL logical");

    G4LogicalVolume* lRefconeEqLoCutLogical = new G4LogicalVolume(lRefConeEqLoCutSolid,
        mData->GetMaterial("NoOptic_Reflector"),
        "RefConeType3 ETEL logical");
    G4LogicalVolume* lLEDHoleAirLogical = new G4LogicalVolume (lLEDHoleAirSolid, 
        mData->GetMaterial("Ri_Vacuum"), 
        "LED hole");
    G4LogicalVolume* lLEDGlassTopLogical = new G4LogicalVolume (lLEDGlassTopSolid, 
        mData->GetMaterial("RiAbs_Glass_Vitrovex"), 
        "LED Glass top logical");
    G4LogicalVolume* lLEDLogical = new G4LogicalVolume (lLEDSolid, 
        mData->GetMaterial("Ri_Glass_LED"), 
        "LED logical");

    //Placements
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lSupStructureLogical, "SupportStructure_physical", lGelLogical, false, 0, mCheckOverlaps);

    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lGelLogical, "Gel_physical", lGlassLogical, false, 0, mCheckOverlaps);

    G4Transform3D lTransformers;
    std::stringstream lConverter;
    for (int k = 0; k <= mTotalNrPMTs - 1; k++)
    {
        lConverter.str("");
        lConverter << "_" << k ;

        lTransformers = G4Transform3D(mPMTRotations[k], mPMTPositions[k]);
        mPMTManager->PlaceIt(lTransformers, lGelLogical, lConverter.str());
 
        //Placing reflective cones:
        lConverter.str("");
        lConverter << "RefCone_" << k << "_physical";
        lTransformers = G4Transform3D(mPMTRotations[k], mReflectorPositions[k]);
        if (k >= mNrPolarPMTs && k <= mNrPolarPMTs + mNrEqPMTs - 1)
        {
            lTransformers = G4Transform3D(mPMTRotPhi[k], G4ThreeVector(0, 0, 0));
            new G4PVPlacement(lTransformers, lRefconeEqUpCutLogical, lConverter.str(), lGelLogical, false, 0, mCheckOverlaps);
        }
        else if (k >= mNrPolarPMTs + mNrEqPMTs && k <= mNrPolarPMTs + mNrEqPMTs * 2 - 1)
        {
            lTransformers = G4Transform3D(mPMTRotPhi[k], G4ThreeVector(0, 0, 0));
            new G4PVPlacement(lTransformers, lRefconeEqLoCutLogical, lConverter.str(), lGelLogical, false, 0, mCheckOverlaps);
        }
        else
        {
            new G4PVPlacement(lTransformers, lRefConePolarLogical, lConverter.str(), lGelLogical, false, 0, mCheckOverlaps);
        }
    }
    //Place LED
    // Placement LED itself
	new G4PVPlacement (0, G4ThreeVector(0,0,-1.7*mm), lLEDLogical, "LED_physical", lLEDHoleAirLogical, false, 0, mCheckOverlaps); // LED to glass 1mm: -1.7mm; LED to glass 0.6mm: -1.1mm
	// Placement LED glass top between air (where LED is) and gel
	new G4PVPlacement (0, G4ThreeVector(0,0,4.4*mm+0.875*mm), lLEDGlassTopLogical, "LED_Glass top_physical", lLEDHoleAirLogical, false, 0, mCheckOverlaps);
    //Place LED holes (air), with LEDs inside
	for (int k = 0; k <=mNrTotalLED-1; k++) { 
        lConverter.str("");
        lConverter << "LEDhole_physical_" << k;
      	//new G4PVPlacement (mLEDTransformers[k], lLEDHoleAirLogical, lConverter.str(), lGelLogical, false, 0, mCheckOverlaps);
    }
        
        
        
    // ------------------ Add outer shape solid to MultiUnion in case you need substraction -------------------------------------------

    AppendComponent(lGlassSolid, lGlassLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PressureVessel");
    // ------------------- optical border surfaces --------------------------------------------------------------------------------
    new G4LogicalSkinSurface("RefCone_skin", lRefConePolarLogical,
        mData->GetOpticalSurface("Refl_V95Gel"));
    new G4LogicalSkinSurface("RefCone_skin", lRefconeEqUpCutLogical,
        mData->GetOpticalSurface("Refl_V95Gel"));
    new G4LogicalSkinSurface("RefCone_skin", lRefconeEqLoCutLogical,
        mData->GetOpticalSurface("Refl_V95Gel"));


    //     // ---------------- visualisation attributes --------------------------------------------------------------------------------
    lGlassLogical->SetVisAttributes(mGlassVis);
    lGelLogical->SetVisAttributes(mGelVis);
    lSupStructureLogical->SetVisAttributes(mAbsorberVis);
    //lSupStructureLogical->SetVisAttributes(mInvisibleVis);
    lRefConePolarLogical->SetVisAttributes(mAluVis);
    lRefconeEqUpCutLogical->SetVisAttributes(mAluVis);
    lRefconeEqLoCutLogical->SetVisAttributes(mAluVis);
    lLEDHoleAirLogical->SetVisAttributes(mInvisibleVis);
	lLEDLogical->SetVisAttributes(mLEDvis); 
    lLEDGlassTopLogical->SetVisAttributes(mGlassVis); 
}



G4UnionSolid* mDOM::PressureVessel(const G4double pOutRad, G4String pSuffix)
{
    G4Ellipsoid* lTopSolid = new G4Ellipsoid("SphereTop solid" + pSuffix, pOutRad, pOutRad, pOutRad, -5 * mm, pOutRad + 5 * mm);
    G4Ellipsoid* lBottomSolid = new G4Ellipsoid("SphereBottom solid" + pSuffix, pOutRad, pOutRad, pOutRad, -(pOutRad + 5 * mm), 5 * mm);

    G4double zCorners[] = { mCylHigh * 1.001, mCylHigh, 0, -mCylHigh, -mCylHigh * 1.001 };
    G4double rCorners[] = { 0, pOutRad, pOutRad + mCylHigh * sin(mCylinderAngle), pOutRad, 0 };
    G4Polycone* lCylinderSolid = new G4Polycone("Cylinder solid" + pSuffix, 0, 2 * CLHEP::pi, 5, rCorners, zCorners);

    G4UnionSolid* lTempUnion = new G4UnionSolid("temp" + pSuffix, lCylinderSolid, lTopSolid, 0, G4ThreeVector(0, 0, mCylHigh));
    G4UnionSolid* lUnionSolid = new G4UnionSolid("OM body" + pSuffix, lTempUnion, lBottomSolid, 0, G4ThreeVector(0, 0, -mCylHigh));
    return lUnionSolid;
}

std::tuple<G4SubtractionSolid*, G4UnionSolid*> mDOM::SupportStructure()
{
    const G4double lRefConeToHolder = mData->GetValue(mDataKey, "jRefConeToHolder");             // horizontal distance from K??rcher's construction
    G4VSolid* lPMTsolid = mPMTManager->GetPMTSolid();

    //  PMT support structure primitives & cutting "nests" for PMTs later

    G4Ellipsoid* lSupStructureTopSolid = new G4Ellipsoid("FoamSphereTop solid", mSupStructureRad, mSupStructureRad, mSupStructureRad, -5 * mm, mSupStructureRad + 5 * mm);
    G4Ellipsoid* lSupStructureBottomSolid = new G4Ellipsoid("FoamSphereBottom solid", mSupStructureRad, mSupStructureRad, mSupStructureRad, -(mSupStructureRad + 5 * mm), 5 * mm);
    G4Tubs* lSupStructureCylinderSolid = new G4Tubs("FoamCylinder solid", 0, mGlassOutRad - mGlassThick - mGelThickness, mCylHigh, 0, 2 * CLHEP::pi);
    G4UnionSolid* lSupStructureTempUnion = new G4UnionSolid("Foam TempUnion solid", lSupStructureCylinderSolid, lSupStructureTopSolid, 0, G4ThreeVector(0, 0, (mCylHigh)));
    G4UnionSolid* lSupStructureFirstSolid = new G4UnionSolid("Foam solid", lSupStructureTempUnion, lSupStructureBottomSolid, 0, G4ThreeVector(0, 0, -(mCylHigh)));

    //Reflectors nests for support structure substraction
    G4double lRefConeOuterNegRad = mRefConeIdealInRad + lRefConeToHolder / cos(mRefConeAngle);
    G4double lRefConeOuterPosRad = mRefConeIdealInRad + lRefConeToHolder / cos(mRefConeAngle) + 2 * 1.5 * mRefConeHalfZ * tan(mRefConeAngle);
    G4Cons* lRefConeNestConeSolid = new G4Cons("RefConeNestCone", 0, lRefConeOuterNegRad, 0, lRefConeOuterPosRad, 1.5 * mRefConeHalfZ, 0, 2 * CLHEP::pi);
    G4UnionSolid* lRefConeNestSolid = new G4UnionSolid("RefConeNest", lPMTsolid, lRefConeNestConeSolid, 0, G4ThreeVector(0, 0, 1.5 * mRefConeHalfZ));

    //Support structure substraction
    G4SubtractionSolid* lSupStructureSolid;
    G4Transform3D lTransformers;
    for (int k = 0; k <= mTotalNrPMTs - 1; k++)
    {
        lTransformers = G4Transform3D(mPMTRotations[k], mPMTPositions[k]);
        if (k == 0) lSupStructureSolid = new G4SubtractionSolid("TubeHolder solid", lSupStructureFirstSolid, lRefConeNestSolid, lTransformers);
        else lSupStructureSolid = new G4SubtractionSolid("TubeHolder solid", lSupStructureSolid, lRefConeNestSolid, lTransformers);
    }
    return std::make_tuple(lSupStructureSolid, lSupStructureFirstSolid);
}


G4SubtractionSolid* mDOM::EquatorialReflector(G4VSolid* pSupportStructure, G4Cons* pReflCone, G4double pAngle, G4String pSuffix)
{
    G4double lRefConeR = mGlassInRad - mGelThicknessFrontPMT - mPMToffset + mRefConeHalfZ + mEqPMTrOffset;
    G4double lRho = lRefConeR * sin(pAngle);
    G4double lX = lRho * cos(0 * deg);
    G4double lY = lRho * sin(0 * deg);
    G4int lSign;
    if (pAngle == mThetaEquatorial) lSign = 1; //if upper reflector
    else lSign = -1; 
    G4double lZ = lRefConeR * cos(pAngle) + lSign * ( mCylHigh - mEqPMTzOffset );

    G4RotationMatrix* lRot = new G4RotationMatrix();
    lRot->rotateY(pAngle);
    G4Transform3D lTransformers = G4Transform3D(*lRot, G4ThreeVector(lX, lY, lZ));
    G4IntersectionSolid* lUncutReflector = new G4IntersectionSolid("RefConeType2_" + pSuffix, pSupportStructure, pReflCone, lTransformers);

    // Start cutting process
    G4Cons* lRefConCutterSolid = new G4Cons("RefConeCutter" + pSuffix ,
        0, mRefConeIdealInRad + mRefConeSheetThickness * std::sqrt(2.) + 2 * mm,
        0, mRefConeIdealInRad + mRefConeSheetThickness * std::sqrt(2.) + 2 * mRefConeHalfZ + 2 * mm,
        mRefConeHalfZ, 0, 2 * CLHEP::pi);

    G4double lCutX = lRho * cos(360. * deg / mNrEqPMTs);
    G4double lCutY = lRho * sin(360. * deg / mNrEqPMTs);

    lRot = new G4RotationMatrix();
    lRot->rotateY(pAngle);
    lRot->rotateZ(360. * deg / mNrEqPMTs);
    lTransformers = G4Transform3D(*lRot, G4ThreeVector(lCutX, lCutY, lZ));
    G4SubtractionSolid* lRefConeCut = new G4SubtractionSolid("RefConeEqUpCut" + pSuffix ,
        lUncutReflector,
        lRefConCutterSolid,
        lTransformers);
        
    lRot = new G4RotationMatrix();
    lRot->rotateY(pAngle);
    lRot->rotateZ(- 360. * deg / mNrEqPMTs);
    lTransformers = G4Transform3D(*lRot, G4ThreeVector(lCutX, -lCutY, lZ));
    lRefConeCut = new G4SubtractionSolid("RefConeEqLoCut" + pSuffix, lRefConeCut, lRefConCutterSolid, lTransformers);

    return lRefConeCut;
}

G4SubtractionSolid* mDOM::SubstractHarnessPlug(G4VSolid* pSolid) {
    Component Plug = mHarness->GetComponent("Plug");
    G4Transform3D lPlugTransform = G4Transform3D(Plug.Rotation, Plug.Position);
    G4SubtractionSolid* lSolidSubstracted = new G4SubtractionSolid(pSolid->GetName() + "_plugSubstracted", pSolid, Plug.VSolid, lPlugTransform);
    return lSolidSubstracted;
}

std::tuple<G4SubtractionSolid*, G4UnionSolid*, G4UnionSolid*, G4Tubs*> mDOM::LedFlashers(G4VSolid* lSupStructureSolid){

    G4SubtractionSolid* lSupStructureSolidSub;

	// cut in holding structure to place afterwards LEDs inside 
    //NOTE: This should not be necessary (substraction with lAirSolid should be enough), but somehow a visual glitch is visible otherwise
	G4Cons* lCutTubeHolderInnerSolid = 	new G4Cons("cut inner solid", 0, 2.65*mm, 0, 4.288*mm, 4.4*mm, 0, 2*CLHEP::pi); // inner cone
	G4Tubs* lCutTubeHolderOuterSolid = 	new G4Tubs("cut outer solid", 0, 7.25*mm, 2*0.667*mm, 0, 2*CLHEP::pi); 				// outer cone
	G4UnionSolid* lCutTubeHolderSolid = new G4UnionSolid("cut solid", lCutTubeHolderInnerSolid, lCutTubeHolderOuterSolid, 0, G4ThreeVector(0, 0, 4.4*mm+0.667*mm)); // union

	// extra cone only(!) for air between LED and holding structure; fits exactly in cut in holding structure
	G4Cons* lAirInnerSolid =  new G4Cons("air inner solid", 0, 2.65*mm, 0, 4.288*mm, 4.4*mm,0 , 2*CLHEP::pi); // inner cone
	G4Tubs* lAirOuterSolid =  new G4Tubs("air outer solid", 0, 7.25*mm, 0.875*mm,0 , 2*CLHEP::pi); // outer cone
	G4UnionSolid* lAirSolid = new G4UnionSolid("air Solid", lAirInnerSolid, lAirOuterSolid, 0, G4ThreeVector(0, 0, 4.4*mm+0.875*mm)); // union
	
	// glass on top of LED 
	G4Tubs* lLEDGlassTopSolid = new G4Tubs("LED Glass top solid", 0, 7.25*mm, 0.875*mm, 0, 2*CLHEP::pi); // glass thickness is 1.75
	
	// LED itself
	G4Ellipsoid* lLEDTopSolid = new G4Ellipsoid("LEDTop solid", 0.25*cm, 0.25*cm, 0.25*cm, 0, 0.25*cm); // LED diameter=2.5mm
	G4Tubs* lLEDCylSolid = 		new G4Tubs("LEDCyl solid", 0, 0.25*cm, 0.26*cm, 0, 2*CLHEP::pi);
	G4UnionSolid* lLEDSolid = 	new G4UnionSolid("temp", lLEDCylSolid, lLEDTopSolid, 0, G4ThreeVector(0, 0, 0.26*cm));
	
	// subtraction holding structure
    for (int k = 0; k <= mNrTotalLED-1; k++) {    
		if (k==0){
            lSupStructureSolidSub = new G4SubtractionSolid("TubeHolderSub solid", lSupStructureSolid, lCutTubeHolderSolid, mLEDTransformers[k]);
			} 		
        else { 
            lSupStructureSolidSub = new G4SubtractionSolid("TubeHolderSub solid", lSupStructureSolidSub, lCutTubeHolderSolid, mLEDTransformers[k]);
		}
    }

    //
    return std::make_tuple(lSupStructureSolidSub, lLEDSolid, lAirSolid, lLEDGlassTopSolid);
}

void mDOM::SetPMTPositions()
{

    G4double lPMTr;     // radius for PMT positioning
    G4double lRefConeR; // radius for RefCone positioning
    G4double lPMTzOffset;
    G4RotationMatrix lRot;
    G4RotationMatrix lRot2;
    G4double lPMTtheta;
    G4double lPMTphi;

    for (int i = 0; i <= mTotalNrPMTs - 1; i++)
    {
        lRot = G4RotationMatrix();
        lRot2 = G4RotationMatrix();
        lPMTr = mGlassInRad - mGelThicknessFrontPMT - mPMToffset;
        lRefConeR = mGlassInRad - mGelThicknessFrontPMT - mPMToffset + mRefConeHalfZ;

        if (i >= 0 && i <= mNrPolarPMTs - 1)
        {
            lPMTtheta = mThetaPolar;
            lPMTphi = i * 360. * deg / mNrPolarPMTs;
            lPMTzOffset = mCylHigh;
        }
        if (i >= mNrPolarPMTs && i <= mNrPolarPMTs + mNrEqPMTs - 1)
        {
            lPMTtheta = mThetaEquatorial;
            lPMTphi = (i - mNrPolarPMTs + 0.5) * 360. * deg / mNrEqPMTs;
            lPMTzOffset = mCylHigh - mEqPMTzOffset;
            lPMTr += mEqPMTrOffset;
            lRefConeR += mEqPMTrOffset;
        }
        if (i >= mNrPolarPMTs + mNrEqPMTs && i <= mNrPolarPMTs + 2 * mNrEqPMTs - 1)
        {
            lPMTtheta = 180. * deg - mThetaEquatorial;
            lPMTphi = (i - (mNrPolarPMTs + mNrEqPMTs) + 0.5) * 360. * deg / mNrEqPMTs;
            lPMTzOffset = -mCylHigh + mEqPMTzOffset;
            lPMTr += mEqPMTrOffset;
            lRefConeR += mEqPMTrOffset;
        }
        if (i >= mNrPolarPMTs + 2 * mNrEqPMTs && i <= mTotalNrPMTs - 1)
        {
            lPMTtheta = 180. * deg - mThetaPolar;
            lPMTphi = (i - (mNrPolarPMTs + 2 * mNrEqPMTs)) * 360. * deg / mNrPolarPMTs;
            lPMTzOffset = -mCylHigh;
        }

        G4double lPMTrho = lPMTr * sin(lPMTtheta);
        mPMTPositions.push_back(G4ThreeVector(lPMTrho * cos(lPMTphi), lPMTrho * sin(lPMTphi), lPMTr * cos(lPMTtheta) + lPMTzOffset));

        lRot.rotateY(lPMTtheta);
        lRot.rotateZ(lPMTphi);
        lRot2.rotateZ(lPMTphi);
        mPMTRotations.push_back(lRot);
        mPMTRotPhi.push_back(lRot2);

        G4double lRefConeRho = lRefConeR * sin(lPMTtheta);
        mReflectorPositions.push_back(G4ThreeVector(lRefConeRho * cos(lPMTphi), lRefConeRho * sin(lPMTphi), lRefConeR * cos(lPMTtheta) + lPMTzOffset));
    }
}

void mDOM::SetLEDPositions()
{
    G4int lNrPolLED = 1;										
    G4int lNrEqLED = 4;	
    mNrTotalLED =(lNrPolLED+lNrEqLED)*2;	// 10 LEDs in total
    G4double lLEDr = 152.8*mm;
    G4double lLEDrho;
    G4double lLEDtheta;
    G4double lLEDphi;
    G4double lLEDx;
    G4double lLEDy;
    G4double lLEDz;
    G4double lThetaEqLED = 61*deg; // 61 upper sphere, 180-61 lower sphere
    G4double lThetaPolLED = 8.2*deg; // 8.2 upper sphere, 180-8.2 lower sphere
    //G4double lThetaPolLED = 0*deg; // test
    G4double lPMTzOffset;
    G4double lPolEqPMTPhiPhase = mData->GetValue(mDataKey, "jPolEqPMTPhiPhase");
    G4ThreeVector lLEDPosition;
	G4RotationMatrix* lLEDRotation;
    G4Transform3D lTransformers;

    mLED_AngFromSphere.resize(mNrTotalLED);

	for (int i = 0; i<=mNrTotalLED-1; i++) {
		if (i>=0 && i<=lNrPolLED-1){	// pol upper -> i=0
            lLEDtheta=lThetaPolLED;
            lLEDphi=(lPolEqPMTPhiPhase+i*360.*deg/lNrPolLED);
			lPMTzOffset = mCylHigh;
			}
		if (i>=lNrPolLED && i<=lNrEqLED){ 	// eq upper -> i={1;4}
            lLEDtheta=lThetaEqLED;
            lLEDphi=lPolEqPMTPhiPhase+(i-1)*360.*deg/lNrEqLED;
			lPMTzOffset = mCylHigh;
			}
		if (i>=lNrEqLED+1 && i<=mNrTotalLED-2){	// eq lower -> i={5;8}
            lLEDtheta=180*deg-lThetaEqLED;
            lLEDphi=lPolEqPMTPhiPhase+(i-5)*360.*deg/lNrEqLED;
			lPMTzOffset = -mCylHigh;
			}
        
		if (i>=2*lNrEqLED+1 && i<=mNrTotalLED-1){	// pol lower -> i=9
            lLEDtheta=180*deg-lThetaPolLED;
            lLEDphi=lPolEqPMTPhiPhase;
			lPMTzOffset = -mCylHigh;
			}

        lLEDrho	 = lLEDr * sin(lLEDtheta);

        (mLED_AngFromSphere.at(i)).resize(3);
        (mLED_AngFromSphere.at(i)).at(0) = lLEDrho;
        (mLED_AngFromSphere.at(i)).at(1) = lLEDtheta;
        (mLED_AngFromSphere.at(i)).at(2) = lLEDphi;

        lLEDx = lLEDrho * cos(lLEDphi);
        lLEDy = lLEDrho * sin(lLEDphi);
        lLEDz = lLEDr * cos(lLEDtheta) + lPMTzOffset;

        lLEDPosition = G4ThreeVector(lLEDx, lLEDy, lLEDz);
        lLEDRotation= new G4RotationMatrix();
        lLEDRotation->rotateY(lLEDtheta);
        lLEDRotation->rotateZ(lLEDphi);

        lTransformers = G4Transform3D(*lLEDRotation, lLEDPosition);
        mLEDTransformers.push_back(lTransformers);

    }
}
