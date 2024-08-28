
#include "OMSimPDOM.hh"
#include "OMSimLogger.hh"
#include "OMSimCommandArgsTable.hh"
#include <G4Ellipsoid.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4Orb.hh>
#include <G4Polycone.hh>

pDOM::pDOM(G4bool p_placeHarness): OMSimOpticalModule(new OMSimPMTConstruction()), m_placeHarness(p_placeHarness)
{
    log_info("Constructing pDOM");
    m_managerPMT->selectPMT("pmt_Hamamatsu_R7081");
    m_managerPMT->construction();
    construction();
}


void pDOM::construction()
{
    m_components.clear();

    G4VSolid *solidPMT = m_managerPMT->getPMTSolid();

    G4double m_gelThickness = 10 * mm;

    G4double zPMT = 0.5 * 12 * 25.4 * mm - m_managerPMT->getDistancePMTCenterToTip() - m_gelThickness;

    G4Orb *glassSphereSolid = new G4Orb("PDOM_GlassSphere solid", 0.5 * 13 * 25.4 * mm);
    G4Orb *gelSphereSolid = new G4Orb("PDOM_GelSphere solid", 0.5 * 12 * 25.4 * mm);

    G4Ellipsoid *airAuxSolid = new G4Ellipsoid("PDOM_AirAux solid", 0.5 * 12 * 25.4 * mm, 0.5 * 12 * 25.4 * mm, 0.5 * 12 * 25.4 * mm, -0.5 * 13 * 25.4 * mm, 50 * mm);

    G4SubtractionSolid *airSolid = new G4SubtractionSolid("PDOM_Air solid", airAuxSolid, solidPMT, 0, G4ThreeVector(0, 0, zPMT));
    G4Tubs *boardSolid = new G4Tubs("PDOM_Board solid", 52 * mm, 0.5 * 11 * 25.4 * mm, 2 * mm, 0, 2 * CLHEP::pi);
    G4Tubs *baseSolid = new G4Tubs("PDOM_Board solid", 0 * mm, 6 * cm, 2 * mm, 0, 2 * CLHEP::pi);

    // Harness
    G4double harnessInner[] = {0, 0, 0, 0};
    G4double harnessRadii[] = {(0.5 * 365.76 - 8.3) * mm, 0.5 * 365.76 * mm, 0.5 * 365.76 * mm, (0.5 * 365.76 - 8.3) * mm};
    G4double harnessZplanes[] = {-31.75 * mm, -10 * mm, 10 * mm, 31.75 * mm};
    G4Polycone *harnessAuxSolid = new G4Polycone("PDOM_HarnessAux solid", 0, 2 * CLHEP::pi, 4, harnessZplanes, harnessInner, harnessRadii);
    G4SubtractionSolid *harnessSolid = new G4SubtractionSolid("PDOM_Harness solid", harnessAuxSolid, glassSphereSolid);

    // Logicals mData
    G4LogicalVolume *glassSphereLogical = new G4LogicalVolume(glassSphereSolid,
                                                               m_data->getMaterial("RiAbs_Glass_Benthos"),
                                                               "PDOM_Glass logical");
    G4LogicalVolume *harnessLogical = new G4LogicalVolume(harnessSolid,
                                                           m_data->getMaterial("NoOptic_Stahl"),
                                                           "PDOM_Harness logical");

    G4LogicalVolume *gelLogical = new G4LogicalVolume(gelSphereSolid,
                                                       m_data->getMaterial("RiAbs_Gel_QGel900"),
                                                       "PDOM_Gel logical");

    G4LogicalVolume *boardLogical = new G4LogicalVolume(boardSolid,
                                                         m_data->getMaterial("NoOptic_Absorber"),
                                                         "PDOM_Board logical");

    G4LogicalVolume *baseLogical = new G4LogicalVolume(baseSolid,
                                                        m_data->getMaterial("NoOptic_Absorber"),
                                                        "PDOM_Base logical");

    G4LogicalVolume *airLogical = new G4LogicalVolume(airSolid,
                                                       m_data->getMaterial("Ri_Vacuum"),
                                                       "PDOM_Air logical");

    G4PVPlacement *boardPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, -40 * mm), boardLogical, "pDOMBoardPhys", airLogical, false, 0, m_checkOverlaps);
    G4PVPlacement *basePhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, -105 * mm), baseLogical, "pDOMBasePhys", airLogical, false, 0, m_checkOverlaps);

    G4PVPlacement *airPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), airLogical, "pDOMAirPhys ", gelLogical, false, 0, m_checkOverlaps);

    m_managerPMT->placeIt(G4ThreeVector(0, 0, zPMT), G4RotationMatrix(), gelLogical, "_0");

    G4PVPlacement *gelPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), gelLogical, "pDOMGelPhys", glassSphereLogical, false, 0, m_checkOverlaps);

    if (m_placeHarness)
        appendComponent(harnessSolid, harnessLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "pDOM_Harness");

    appendComponent(glassSphereSolid, glassSphereLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "pDOM_" + std::to_string(m_index));

    // ------------------- optical border surfaces --------------------------------------------------------------------------------
    new G4LogicalSkinSurface("PDOM_Harness_skin", harnessLogical, m_data->getOpticalSurface("Surf_StainlessSteelGround"));

    glassSphereLogical->SetVisAttributes(m_glassVis);
    harnessLogical->SetVisAttributes(m_steelVis);
    gelLogical->SetVisAttributes(m_gelVis);
    airLogical->SetVisAttributes(m_airVis);
    boardLogical->SetVisAttributes(m_boardVis);
    baseLogical->SetVisAttributes(m_boardVis);
}
