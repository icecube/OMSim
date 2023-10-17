/**
 *  @todo - Write documentation and parse current comments into Doxygen style
 */
#include "OMSimSensitiveDetector.hh"
#include "OMSimMDOM.hh"
#include "OMSimMDOMHarness.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimLogger.hh"

#include <G4Ellipsoid.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4Polycone.hh>

mDOM::~mDOM()
{
    delete mFlashers;
    if (mPlaceHarness)
    {
        delete mHarness;
    }
}
mDOM::mDOM(InputDataManager *pData, G4bool pPlaceHarness, G4int pIndex)
{
    log_info("Constructing mDOM");

    mCheckOverlaps = OMSimCommandArgsTable::getInstance().get<bool>("check_overlaps");
    mPlaceHarness = pPlaceHarness;
    mData = pData;
    mIndex = pIndex;
    mPMTManager = new OMSimPMTConstruction(mData);
    mFlashers = new mDOMFlasher(mData);

    mPMTManager->selectPMT("pmt_Hamamatsu_R15458_20nm");
    mPMTManager->construction();
    mRefConeIdealInRad = mPMTManager->getMaxPMTRadius() + 2 * mm;
    mPMToffset = mPMTManager->getDistancePMTCenterToTip();

    if (mPlaceHarness)
    {
        mHarness = new mDOMHarness(this, mData);
        integrateDetectorComponent(mHarness, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "");
    }

    construction();
    log_debug("Finished constructing mDOM");
}

void mDOM::construction()
{

    setPMTPositions();
    setLEDPositions();

    G4UnionSolid *lGlassSolid = pressureVessel(mGlassOutRad, "Glass");
    G4UnionSolid *lGelSolid = pressureVessel(mGlassInRad, "Gel");

    G4SubtractionSolid *lSupStructureSolid;
    G4UnionSolid *lSupStructureFirstSolid;

    std::tie(lSupStructureSolid, lSupStructureFirstSolid) = supportStructure();

    // Reflector solid
    G4Cons *lRefConeBasicSolid = new G4Cons("RefConeBasic", mRefConeIdealInRad,
                                            mRefConeIdealInRad + mRefConeSheetThickness / cos(mRefConeAngle),
                                            mRefConeIdealInRad + 2 * mRefConeHalfZ * tan(mRefConeAngle), mRefConeIdealInRad + mRefConeSheetThickness / cos(mRefConeAngle) + 2 * mRefConeHalfZ * tan(mRefConeAngle), mRefConeHalfZ, 0, 2 * CLHEP::pi);

    // Reflector cone for polar PMTs
    G4Ellipsoid *lSupStructureTopSolid = new G4Ellipsoid("FoamSphereTop solid", mSupStructureRad, mSupStructureRad, mSupStructureRad, -5 * mm, mSupStructureRad + 5 * mm);
    G4IntersectionSolid *lRefConePolarSolid = new G4IntersectionSolid("PolarRefCones", lRefConeBasicSolid, lSupStructureTopSolid, 0, G4ThreeVector(0, 0, -(mGlassInRad - mGelThicknessFrontPMT - mPMToffset + mRefConeHalfZ)));

    // Reflector cone for equatorial PMTs
    G4SubtractionSolid *lRefConeEqUpCutSolid = equatorialReflector(lSupStructureFirstSolid, lRefConeBasicSolid, mThetaEquatorial, "upper");
    G4SubtractionSolid *lRefConeEqLoCutSolid = equatorialReflector(lSupStructureFirstSolid, lRefConeBasicSolid, 180. * deg - mThetaEquatorial, "lower");

    // LED flashers
    lSupStructureSolid = substractFlashers(lSupStructureSolid);

    // Logicals
    G4LogicalVolume *lGelLogical = new G4LogicalVolume(lGelSolid,
                                                       mData->getMaterial("RiAbs_Gel_Shin-Etsu"),
                                                       "Gelcorpus logical");

    G4LogicalVolume *lSupStructureLogical = new G4LogicalVolume(lSupStructureSolid,
                                                                mData->getMaterial("NoOptic_Absorber"),
                                                                "TubeHolder logical");

    G4LogicalVolume *lGlassLogical = new G4LogicalVolume(lGlassSolid,
                                                         mData->getMaterial("RiAbs_Glass_Vitrovex"),
                                                         "Glass_log");
    if (mPlaceHarness)
    {
        lGelLogical = new G4LogicalVolume(substractHarnessPlug(lGelSolid),
                                          mData->getMaterial("RiAbs_Gel_Shin-Etsu"),
                                          "Gelcorpus logical");
        lSupStructureLogical = new G4LogicalVolume(substractHarnessPlug(lSupStructureSolid),
                                                   mData->getMaterial("NoOptic_Absorber"),
                                                   "TubeHolder logical");
        lGlassLogical = new G4LogicalVolume(substractHarnessPlug(lGlassSolid),
                                            mData->getMaterial("RiAbs_Glass_Vitrovex"),
                                            "Glass_log");
    }
    G4LogicalVolume *lRefConePolarLogical = new G4LogicalVolume(lRefConePolarSolid,
                                                                mData->getMaterial("NoOptic_Reflector"),
                                                                "RefConeType1 logical");

    G4LogicalVolume *lRefconeEqUpCutLogical = new G4LogicalVolume(lRefConeEqUpCutSolid,
                                                                  mData->getMaterial("NoOptic_Reflector"),
                                                                  "RefConeType2 ETEL logical");

    G4LogicalVolume *lRefconeEqLoCutLogical = new G4LogicalVolume(lRefConeEqLoCutSolid,
                                                                  mData->getMaterial("NoOptic_Reflector"),
                                                                  "RefConeType3 ETEL logical");

    // Placements
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lSupStructureLogical, "SupportStructure_physical", lGelLogical, false, 0, mCheckOverlaps);

    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lGelLogical, "Gel_physical", lGlassLogical, false, 0, mCheckOverlaps);

    G4Transform3D lTransformers;
    std::stringstream lConverter;
    for (int k = 0; k <= mTotalNrPMTs - 1; k++)
    {
        lConverter.str("");
        lConverter << "_" << k;

        lTransformers = G4Transform3D(mPMTRotations[k], mPMTPositions[k]);
        mPMTManager->placeIt(lTransformers, lGelLogical, lConverter.str());

        // Placing reflective cones:
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

    // Place LED
    for (int k = 0; k <= mNrTotalLED - 1; k++)
    {
        lConverter.str("");
        lConverter << "LEDhole_physical_" << k;
        mFlashers->placeIt(mLEDTransformers[k], lGelLogical, lConverter.str());
    }

    // ------------------ Add outer shape solid to MultiUnion in case you need substraction -------------------------------------------

    appendComponent(lGlassSolid, lGlassLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PressureVessel_" + std::to_string(mIndex));
    // ------------------- optical border surfaces --------------------------------------------------------------------------------
    new G4LogicalSkinSurface("RefCone_skin", lRefConePolarLogical,
                             mData->getOpticalSurface("Refl_V95Gel"));
    new G4LogicalSkinSurface("RefCone_skin", lRefconeEqUpCutLogical,
                             mData->getOpticalSurface("Refl_V95Gel"));
    new G4LogicalSkinSurface("RefCone_skin", lRefconeEqLoCutLogical,
                             mData->getOpticalSurface("Refl_V95Gel"));

    //     // ---------------- visualisation attributes --------------------------------------------------------------------------------
    lGlassLogical->SetVisAttributes(mGlassVis);
    lGelLogical->SetVisAttributes(mGelVis);
    lSupStructureLogical->SetVisAttributes(mAbsorberVis);
    // lSupStructureLogical->SetVisAttributes(mInvisibleVis);
    lRefConePolarLogical->SetVisAttributes(mAluVis);
    lRefconeEqUpCutLogical->SetVisAttributes(mAluVis);
    lRefconeEqLoCutLogical->SetVisAttributes(mAluVis);
}

G4UnionSolid *mDOM::pressureVessel(const G4double pOutRad, G4String pSuffix)
{
    G4Ellipsoid *lTopSolid = new G4Ellipsoid("SphereTop solid" + pSuffix, pOutRad, pOutRad, pOutRad, -5 * mm, pOutRad + 5 * mm);
    G4Ellipsoid *lBottomSolid = new G4Ellipsoid("SphereBottom solid" + pSuffix, pOutRad, pOutRad, pOutRad, -(pOutRad + 5 * mm), 5 * mm);

    G4double zCorners[] = {mCylHigh * 1.001, mCylHigh, 0, -mCylHigh, -mCylHigh * 1.001};
    G4double rCorners[] = {0, pOutRad, pOutRad + mCylHigh * sin(mCylinderAngle), pOutRad, 0};
    G4Polycone *lCylinderSolid = new G4Polycone("Cylinder solid" + pSuffix, 0, 2 * CLHEP::pi, 5, rCorners, zCorners);

    G4UnionSolid *lTempUnion = new G4UnionSolid("temp" + pSuffix, lCylinderSolid, lTopSolid, 0, G4ThreeVector(0, 0, mCylHigh));
    G4UnionSolid *lUnionSolid = new G4UnionSolid("OM body" + pSuffix, lTempUnion, lBottomSolid, 0, G4ThreeVector(0, 0, -mCylHigh));
    return lUnionSolid;
}

std::tuple<G4SubtractionSolid *, G4UnionSolid *> mDOM::supportStructure()
{
    G4VSolid *lPMTsolid = mPMTManager->getPMTSolid();

    //  PMT support structure primitives & cutting "nests" for PMTs later

    G4Ellipsoid *lSupStructureTopSolid = new G4Ellipsoid("FoamSphereTop solid", mSupStructureRad, mSupStructureRad, mSupStructureRad, -5 * mm, mSupStructureRad + 5 * mm);
    G4Ellipsoid *lSupStructureBottomSolid = new G4Ellipsoid("FoamSphereBottom solid", mSupStructureRad, mSupStructureRad, mSupStructureRad, -(mSupStructureRad + 5 * mm), 5 * mm);
    G4Tubs *lSupStructureCylinderSolid = new G4Tubs("FoamCylinder solid", 0, mGlassOutRad - mGlassThick - mGelThickness, mCylHigh, 0, 2 * CLHEP::pi);
    G4UnionSolid *lSupStructureTempUnion = new G4UnionSolid("Foam TempUnion solid", lSupStructureCylinderSolid, lSupStructureTopSolid, 0, G4ThreeVector(0, 0, (mCylHigh)));
    G4UnionSolid *lSupStructureFirstSolid = new G4UnionSolid("Foam solid", lSupStructureTempUnion, lSupStructureBottomSolid, 0, G4ThreeVector(0, 0, -(mCylHigh)));

    // Reflectors nests for support structure substraction
    G4double lRefConeOuterNegRad = mRefConeIdealInRad + mRefConeToHolder / cos(mRefConeAngle);
    G4double lRefConeOuterPosRad = mRefConeIdealInRad + mRefConeToHolder / cos(mRefConeAngle) + 2 * 1.5 * mRefConeHalfZ * tan(mRefConeAngle);
    G4Cons *lRefConeNestConeSolid = new G4Cons("RefConeNestCone", 0, lRefConeOuterNegRad, 0, lRefConeOuterPosRad, 1.5 * mRefConeHalfZ, 0, 2 * CLHEP::pi);
    G4UnionSolid *lRefConeNestSolid = new G4UnionSolid("RefConeNest", lPMTsolid, lRefConeNestConeSolid, 0, G4ThreeVector(0, 0, 1.5 * mRefConeHalfZ));

    // Support structure substraction
    G4SubtractionSolid *lSupStructureSolid;
    G4Transform3D lTransformers;
    for (int k = 0; k <= mTotalNrPMTs - 1; k++)
    {
        lTransformers = G4Transform3D(mPMTRotations[k], mPMTPositions[k]);
        if (k == 0)
            lSupStructureSolid = new G4SubtractionSolid("TubeHolder solid", lSupStructureFirstSolid, lRefConeNestSolid, lTransformers);
        else
            lSupStructureSolid = new G4SubtractionSolid("TubeHolder solid", lSupStructureSolid, lRefConeNestSolid, lTransformers);
    }
    return std::make_tuple(lSupStructureSolid, lSupStructureFirstSolid);
}

G4SubtractionSolid *mDOM::equatorialReflector(G4VSolid *pSupportStructure, G4Cons *pReflCone, G4double pAngle, G4String pSuffix)
{
    G4double lRefConeR = mGlassInRad - mGelThicknessFrontPMT - mPMToffset + mRefConeHalfZ + mEqPMTrOffset;
    G4double lRho = lRefConeR * sin(pAngle);
    G4double lX = lRho * cos(0 * deg);
    G4double lY = lRho * sin(0 * deg);
    G4int lSign;
    if (pAngle == mThetaEquatorial)
        lSign = 1; // if upper reflector
    else
        lSign = -1;
    G4double lZ = lRefConeR * cos(pAngle) + lSign * (mCylHigh - mEqPMTzOffset);

    G4RotationMatrix *lRot = new G4RotationMatrix();
    lRot->rotateY(pAngle);
    G4Transform3D lTransformers = G4Transform3D(*lRot, G4ThreeVector(lX, lY, lZ));
    G4IntersectionSolid *lUncutReflector = new G4IntersectionSolid("RefConeType2_" + pSuffix, pSupportStructure, pReflCone, lTransformers);

    // Start cutting process
    G4Cons *lRefConCutterSolid = new G4Cons("RefConeCutter" + pSuffix,
                                            0, mRefConeIdealInRad + mRefConeSheetThickness * std::sqrt(2.) + 2 * mm,
                                            0, mRefConeIdealInRad + mRefConeSheetThickness * std::sqrt(2.) + 2 * mRefConeHalfZ + 2 * mm,
                                            mRefConeHalfZ, 0, 2 * CLHEP::pi);

    G4double lCutX = lRho * cos(360. * deg / mNrEqPMTs);
    G4double lCutY = lRho * sin(360. * deg / mNrEqPMTs);

    lRot = new G4RotationMatrix();
    lRot->rotateY(pAngle);
    lRot->rotateZ(360. * deg / mNrEqPMTs);
    lTransformers = G4Transform3D(*lRot, G4ThreeVector(lCutX, lCutY, lZ));
    G4SubtractionSolid *lRefConeCut = new G4SubtractionSolid("RefConeEqUpCut" + pSuffix,
                                                             lUncutReflector,
                                                             lRefConCutterSolid,
                                                             lTransformers);

    lRot = new G4RotationMatrix();
    lRot->rotateY(pAngle);
    lRot->rotateZ(-360. * deg / mNrEqPMTs);
    lTransformers = G4Transform3D(*lRot, G4ThreeVector(lCutX, -lCutY, lZ));
    lRefConeCut = new G4SubtractionSolid("RefConeEqLoCut" + pSuffix, lRefConeCut, lRefConCutterSolid, lTransformers);

    return lRefConeCut;
}

G4SubtractionSolid *mDOM::substractHarnessPlug(G4VSolid *pSolid)
{
    Component Plug = mHarness->getComponent("Plug");
    G4Transform3D lPlugTransform = G4Transform3D(Plug.Rotation, Plug.Position);
    G4SubtractionSolid *lSolidSubstracted = new G4SubtractionSolid(pSolid->GetName() + "_plugSubstracted", pSolid, Plug.VSolid, lPlugTransform);
    return lSolidSubstracted;
}

G4SubtractionSolid *mDOM::substractFlashers(G4VSolid *lSupStructureSolid)
{

    G4SubtractionSolid *lSupStructureSolidSub;

    // cut in holding structure to place afterwards LEDs inside
    // NOTE: This should not be necessary (substraction with lAirSolid should be enough), but somehow a visual glitch is visible otherwise
    G4Cons *lCutTubeHolderInnerSolid = new G4Cons("cut inner solid", 0, 2.65 * mm, 0, 4.288 * mm, 4.4 * mm, 0, 2 * CLHEP::pi);                                            // inner cone
    G4Tubs *lCutTubeHolderOuterSolid = new G4Tubs("cut outer solid", 0, 7.25 * mm, 2 * 0.667 * mm, 0, 2 * CLHEP::pi);                                                     // outer cone
    G4UnionSolid *lCutTubeHolderSolid = new G4UnionSolid("cut solid", lCutTubeHolderInnerSolid, lCutTubeHolderOuterSolid, 0, G4ThreeVector(0, 0, 4.4 * mm + 0.667 * mm)); // union

    // subtraction holding structure
    for (int k = 0; k <= mNrTotalLED - 1; k++)
    {
        if (k == 0)
        {
            lSupStructureSolidSub = new G4SubtractionSolid("TubeHolderSub solid", lSupStructureSolid, lCutTubeHolderSolid, mLEDTransformers[k]);
        }
        else
        {
            lSupStructureSolidSub = new G4SubtractionSolid("TubeHolderSub solid", lSupStructureSolidSub, lCutTubeHolderSolid, mLEDTransformers[k]);
        }
    }

    return lSupStructureSolidSub;
}

void mDOM::setPMTPositions()
{
    G4double lPMTr;     // radius for PMT positioning
    G4double lRefConeR; // radius for RefCone positioning
    G4double lPMTzOffset;
    G4RotationMatrix lRot;
    G4RotationMatrix lRot2;
    G4double lPMTtheta;
    G4double lPMTphi;

    G4double lPMTrBase = mGlassInRad - mGelThicknessFrontPMT - mPMToffset;
    G4double lRefConeRBase = mGlassInRad - mGelThicknessFrontPMT - mPMToffset + mRefConeHalfZ;

    std::vector<G4double> lPMTthetaList = {mThetaPolar, mThetaEquatorial, 180. * deg - mThetaEquatorial, 180. * deg - mThetaPolar};
    std::vector<G4double> lPMTzOffsetList = {mCylHigh, mCylHigh - mEqPMTzOffset, -mCylHigh + mEqPMTzOffset, -mCylHigh};
    std::vector<int> lPMTCountList = {mNrPolarPMTs, mNrEqPMTs, mNrEqPMTs, mNrPolarPMTs};
    std::vector<G4double> lPhaseShiftList = {0., 0.5, 0.5, 0.};

    for (int j = 0; j < 4; j++)
    {
        lPMTtheta = lPMTthetaList[j];
        lPMTzOffset = lPMTzOffsetList[j];
        lPMTr = lPMTrBase;
        lRefConeR = lRefConeRBase;
        if (j >= 1 && j <= 2)
        {
            lPMTr += mEqPMTrOffset;
            lRefConeR += mEqPMTrOffset;
        }

        for (int i = 0; i < lPMTCountList[j]; i++)
        {
            lRot = G4RotationMatrix();
            lRot2 = G4RotationMatrix();
            // lPMTphi = (i + 0.5 * (j % 2)) * 360. * deg / lPMTCountList[j];
            lPMTphi = (i + lPhaseShiftList[j]) * 360. * deg / lPMTCountList[j];

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
}

void mDOM::setLEDPositions()
{
    G4int lNrPolLED = 1;
    G4int lNrEqLED = 4;
    mNrTotalLED = (lNrPolLED + lNrEqLED) * 2; // 10 LEDs in total
    G4double lLEDr = 152.8 * mm;
    G4double lLEDzOffset;
    G4double lLEDtheta;
    G4double lLEDphi;
    G4ThreeVector lLEDPosition;
    G4RotationMatrix *lLEDRotation;
    G4Transform3D lTransformers;

    mLED_AngFromSphere.resize(mNrTotalLED);

    std::vector<G4double> lLEDthetaList = {mThetaPolLED, mThetaEqLED, 180. * deg - mThetaEqLED, 180. * deg - mThetaPolLED};
    std::vector<G4double> lLEDzOffsetList = {mCylHigh, mCylHigh, -mCylHigh, -mCylHigh};
    std::vector<int> lLEDCountList = {lNrPolLED, lNrEqLED, lNrEqLED, lNrPolLED};
    std::vector<G4double> lPhaseShiftList = {-0.25, 0., 0., 0.};

    int lFlasherCounter = 0;
    for (int j = 0; j < 4; j++)
    {
        lLEDtheta = lLEDthetaList[j];
        lLEDzOffset = lLEDzOffsetList[j];

        for (int i = 0; i < lLEDCountList[j]; i++)
        {
            lLEDphi = mPolEqPMTPhiPhase + (i + lPhaseShiftList[j]) * 360. * deg / lLEDCountList[j];

            G4double lLEDrho = lLEDr * sin(lLEDtheta);
            lLEDPosition = G4ThreeVector(lLEDrho * cos(lLEDphi), lLEDrho * sin(lLEDphi), lLEDr * cos(lLEDtheta) + lLEDzOffset);

            (mLED_AngFromSphere.at(lFlasherCounter)).resize(3);
            (mLED_AngFromSphere.at(lFlasherCounter)).at(0) = lLEDrho;
            (mLED_AngFromSphere.at(lFlasherCounter)).at(1) = lLEDtheta;
            (mLED_AngFromSphere.at(lFlasherCounter)).at(2) = lLEDphi;

            lLEDRotation = new G4RotationMatrix();
            lLEDRotation->rotateY(lLEDtheta);
            lLEDRotation->rotateZ(lLEDphi);

            lTransformers = G4Transform3D(*lLEDRotation, lLEDPosition);
            mLEDTransformers.push_back(lTransformers);
            lFlasherCounter++;
        }
    }
}
