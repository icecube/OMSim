
#include "OMSimPDOM.hh"
#include "OMSimLogger.hh"
#include "OMSimCommandArgsTable.hh"
#include <G4Ellipsoid.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4Orb.hh>
#include <G4Polycone.hh>

pDOM::pDOM(InputDataManager *pData, G4bool pPlaceHarness): OMSimOpticalModule(pData, new OMSimPMTConstruction(pData)), mPlaceHarness(pPlaceHarness)
{
    log_info("Constructing pDOM");
    mPMTManager->selectPMT("pmt_Hamamatsu_R7081");
    mPMTManager->construction();
    construction();
}


void pDOM::construction()
{
    mComponents.clear();

    G4VSolid *lPMTsolid = mPMTManager->getPMTSolid();

    G4double mGelThickness = 10 * mm;

    G4double lPMTz = 0.5 * 12 * 25.4 * mm - mPMTManager->getDistancePMTCenterToTip() - mGelThickness;

    G4Orb *lGlassSphereSolid = new G4Orb("PDOM_GlassSphere solid", 0.5 * 13 * 25.4 * mm);
    G4Orb *lGelSphereSolid = new G4Orb("PDOM_GelSphere solid", 0.5 * 12 * 25.4 * mm);

    G4Ellipsoid *lAirAuxSolid = new G4Ellipsoid("PDOM_AirAux solid", 0.5 * 12 * 25.4 * mm, 0.5 * 12 * 25.4 * mm, 0.5 * 12 * 25.4 * mm, -0.5 * 13 * 25.4 * mm, 50 * mm);

    G4SubtractionSolid *lAirSolid = new G4SubtractionSolid("PDOM_Air solid", lAirAuxSolid, lPMTsolid, 0, G4ThreeVector(0, 0, lPMTz));
    G4Tubs *lBoardSolid = new G4Tubs("PDOM_Board solid", 52 * mm, 0.5 * 11 * 25.4 * mm, 2 * mm, 0, 2 * CLHEP::pi);
    G4Tubs *lBaseSolid = new G4Tubs("PDOM_Board solid", 0 * mm, 6 * cm, 2 * mm, 0, 2 * CLHEP::pi);

    // Harness
    G4double lHarnessInner[] = {0, 0, 0, 0};
    G4double lHarnessRadii[] = {(0.5 * 365.76 - 8.3) * mm, 0.5 * 365.76 * mm, 0.5 * 365.76 * mm, (0.5 * 365.76 - 8.3) * mm};
    G4double lHarnessZplanes[] = {-31.75 * mm, -10 * mm, 10 * mm, 31.75 * mm};
    G4Polycone *lHarnessAuxSolid = new G4Polycone("PDOM_HarnessAux solid", 0, 2 * CLHEP::pi, 4, lHarnessZplanes, lHarnessInner, lHarnessRadii);
    G4SubtractionSolid *lHarnessSolid = new G4SubtractionSolid("PDOM_Harness solid", lHarnessAuxSolid, lGlassSphereSolid);

    // Logicals mData
    G4LogicalVolume *lGlassSphereLogical = new G4LogicalVolume(lGlassSphereSolid,
                                                               mData->getMaterial("RiAbs_Glass_Benthos"),
                                                               "PDOM_Glass logical");
    G4LogicalVolume *lHarnessLogical = new G4LogicalVolume(lHarnessSolid,
                                                           mData->getMaterial("NoOptic_Stahl"),
                                                           "PDOM_Harness logical");

    G4LogicalVolume *lGelLogical = new G4LogicalVolume(lGelSphereSolid,
                                                       mData->getMaterial("RiAbs_Gel_QGel900"),
                                                       "PDOM_Gel logical");

    G4LogicalVolume *lBoardLogical = new G4LogicalVolume(lBoardSolid,
                                                         mData->getMaterial("NoOptic_Absorber"),
                                                         "PDOM_Board logical");

    G4LogicalVolume *lBaseLogical = new G4LogicalVolume(lBaseSolid,
                                                        mData->getMaterial("NoOptic_Absorber"),
                                                        "PDOM_Base logical");

    G4LogicalVolume *lAirLogical = new G4LogicalVolume(lAirSolid,
                                                       mData->getMaterial("Ri_Vacuum"),
                                                       "PDOM_Air logical");

    G4PVPlacement *lBoardPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, -40 * mm), lBoardLogical, "pDOMBoardPhys", lAirLogical, false, 0, mCheckOverlaps);
    G4PVPlacement *lBasePhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, -105 * mm), lBaseLogical, "pDOMBasePhys", lAirLogical, false, 0, mCheckOverlaps);

    G4PVPlacement *lAirPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lAirLogical, "pDOMAirPhys ", lGelLogical, false, 0, mCheckOverlaps);

    mPMTManager->placeIt(G4ThreeVector(0, 0, lPMTz), G4RotationMatrix(), lGelLogical);

    G4PVPlacement *lGelPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lGelLogical, "pDOMGelPhys", lGlassSphereLogical, false, 0, mCheckOverlaps);

    if (mPlaceHarness)
        appendComponent(lHarnessSolid, lHarnessLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "pDOM_Harness");

    appendComponent(lGlassSphereSolid, lGlassSphereLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "pDOM_" + std::to_string(mIndex));

    // ------------------- optical border surfaces --------------------------------------------------------------------------------
    new G4LogicalSkinSurface("PDOM_Harness_skin", lHarnessLogical, mData->getOpticalSurface("Refl_StainlessSteelGround"));

    lGlassSphereLogical->SetVisAttributes(mGlassVis);
    lHarnessLogical->SetVisAttributes(mSteelVis);
    lGelLogical->SetVisAttributes(mGelVis);
    lAirLogical->SetVisAttributes(mAirVis);
    lBoardLogical->SetVisAttributes(mBoardVis);
    lBaseLogical->SetVisAttributes(mBoardVis);
}
