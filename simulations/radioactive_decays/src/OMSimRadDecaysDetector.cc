#include "OMSimRadDecaysDetector.hh"
#include "OMSimMDOM.hh"
#include "OMSimPDOM.hh"
#include "OMSimLOM16.hh"
#include "OMSimLOM18.hh"
#include "OMSimDEGG.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include "G4SDManager.hh"
#include "OMSimSensitiveDetector.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4AssemblyVolume.hh"
#include "G4LogicalSkinSurface.hh"
/**
 * @brief Constructs the world volume (sphere).
 */
void OMSimRadDecaysDetector::constructWorld()
{
    m_worldSolid = new G4Orb("World", OMSimCommandArgsTable::getInstance().get<G4double>("world_radius") * m);
    m_worldLogical = new G4LogicalVolume(m_worldSolid, m_data->getMaterial("argWorld"), "World_log", 0, 0, 0);
    m_worldPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), m_worldLogical, "World_phys", 0, false, 0);
    G4VisAttributes *worldVis = new G4VisAttributes(G4Colour(0.45, 0.5, 0.35, 0.));
    m_worldLogical->SetVisAttributes(worldVis);


    G4bool checkOverlaps = true;

    // Pool and water volumes
    G4Box *boxPool = new G4Box("Pool", (346.8/2)*cm, (168.6/2 + 1)*cm, (144.0/2)*cm);
    G4Box *waterSolid = new G4Box("Water", (345.8/2)*cm, (167.6/2)*cm, (143.0/2)*cm);

    // Create pool volume
    G4LogicalVolume *poolLogical = new G4LogicalVolume(boxPool, m_data->getMaterial("NoOptic_Absorber"), "Pool");
    G4PVPlacement *poolPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), 
                                                   poolLogical, "Pool_phys", 
                                                   m_worldLogical, false, 0, checkOverlaps);
    poolLogical->SetVisAttributes(new G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.2)));

    // Create water volume
    m_waterLogical = new G4LogicalVolume(waterSolid, m_data->getMaterial("Ri_Air"), "Water_log");
    m_waterPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.),
                                       m_waterLogical, "Water_phys",
                                       poolLogical, false, 0, checkOverlaps);
    m_waterLogical->SetVisAttributes(new G4VisAttributes(G4Colour(0.0, 0.4, 1.0, 0.2)));

    // Add reflective surface
    new G4LogicalBorderSurface("Water_skin", m_waterPhysical, poolPhysical, 
                              m_data->getOpticalSurface("Surf_Refl_tank_plastic"));
}

/**
 * @brief Constructs the selected detector from the command line argument.
 */
void OMSimRadDecaysDetector::constructDetector()
{
    OMSimHitManager &hitManager = OMSimHitManager::getInstance();
    bool placeHarness = OMSimCommandArgsTable::getInstance().get<bool>("place_harness");

    OMSimOpticalModule *opticalModule = nullptr;

    switch (OMSimCommandArgsTable::getInstance().get<G4int>("detector_type"))
    {

    case 0:
    {
        log_critical("No custom detector implemented!");
        break;
    }
    case 1:
    {
        log_info("Constructing single PMT");
        OMSimPMTConstruction *managerPMT = new OMSimPMTConstruction();
        managerPMT->selectPMT("argPMT");
        managerPMT->construction();
        managerPMT->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), m_worldLogical, "_0");
        hitManager.setNumberOfPMTs(1, 0);
        managerPMT->configureSensitiveVolume(this, "/PMT/0");
        break;
    }
    case 2:
    {
        opticalModule = new mDOM(placeHarness);
        break;
    }
    case 3:
    {

        opticalModule = new DOM(placeHarness);
        break;
    }
    case 4:
    {

        opticalModule = new LOM16(placeHarness);
        break;
    }
    case 5:
    {

        opticalModule = new LOM18(placeHarness);
        break;
    }
    case 6:
    {
        opticalModule = new DEGG(placeHarness);
        break;
    }
        case 7:
    {
        opticalModule = new DOM(placeHarness, true);
        break;
    }
    }

    if (opticalModule)
    {
        log_info("Placing optical module in water volume.");
        // Place in water instead of world
        G4RotationMatrix* rotZ = new G4RotationMatrix();
        rotZ->rotateZ(130*deg);
        opticalModule->placeIt(G4ThreeVector(140*cm, 0, 0), *rotZ, m_waterLogical, "");
        opticalModule->configureSensitiveVolume(this);
        m_opticalModule = opticalModule;
    }
}
