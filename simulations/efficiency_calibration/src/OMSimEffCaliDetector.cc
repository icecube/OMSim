#include "OMSimEffiCaliDetector.hh"
#include "OMSimPDOM.hh"
#include "OMSimLOM16.hh"
#include "OMSimLOM18.hh"
#include "OMSimDEGG.hh"
#include "OMSimMDOM.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include <G4Orb.hh>
#include "OMSimSensitiveDetector.hh"
#include "G4LogicalVolume.hh"
#include "G4Box.hh"

/**
 * @brief Constructs the world volume (sphere).
 */
void OMSimEffiCaliDetector::constructWorld()
{
    m_worldSolid = new G4Orb("World", OMSimCommandArgsTable::getInstance().get<G4double>("world_radius") * m);
    m_worldLogical = new G4LogicalVolume(m_worldSolid, m_data->getMaterial("argWorld"), "World_log", 0, 0, 0);
    m_worldPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), m_worldLogical, "World_phys", 0, false, 0);
    G4VisAttributes *worldVis = new G4VisAttributes(G4Colour(0.45, 0.5, 0.35, 0.));
    m_worldLogical->SetVisAttributes(worldVis);
}

/**
 * @brief Constructs the selected detector from the command line argument and returns the physical world volume.
 * @return Pointer to the physical world volume
 */
void OMSimEffiCaliDetector::constructDetector()
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
        managerPMT->includeHAcoating();
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

        opticalModule = new pDOM(placeHarness);
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
    }

    if (opticalModule)
    {
        opticalModule->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), m_worldLogical, "");
        opticalModule->configureSensitiveVolume(this);
    }
}
