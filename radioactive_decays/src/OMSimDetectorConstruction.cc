#include "OMSimDetectorConstruction.hh"
#include "OMSimPDOM.hh"
#include "OMSimLOM16.hh"
#include "OMSimLOM18.hh"
#include "OMSimDEGG.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"

OMSimDetectorConstruction::OMSimDetectorConstruction()
    : mWorldSolid(0), mWorldLogical(0), mWorldPhysical(0)
{
}

OMSimDetectorConstruction::~OMSimDetectorConstruction()
{
    delete mData;
}

/**
 * @brief Constructs the world volume (sphere).
 */
void OMSimDetectorConstruction::constructWorld()
{
    mWorldSolid = new G4Orb("World", OMSimCommandArgsTable::getInstance().get<G4double>("world_radius") * m);
    mWorldLogical = new G4LogicalVolume(mWorldSolid, mData->getMaterial("argWorld"), "World_log", 0, 0, 0);
    mWorldPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), mWorldLogical, "World_phys", 0, false, 0);
    G4VisAttributes *World_vis = new G4VisAttributes(G4Colour(0.45, 0.5, 0.35, 0.));
    mWorldLogical->SetVisAttributes(World_vis);
}

/**
 * @brief Constructs the selected detector from the command line argument and returns the physical world volume.
 * @return Pointer to the physical world volume
 */
G4VPhysicalVolume *OMSimDetectorConstruction::Construct()
{

    mData = new InputDataManager();
    mData->searchFolders();

    constructWorld();

    OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

    bool lPlaceHarness = OMSimCommandArgsTable::getInstance().get<bool>("place_harness");

    OpticalModule *lOpticalModule;

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
        OMSimPMTConstruction *lPMTManager = new OMSimPMTConstruction(mData);
        lPMTManager->selectPMT("argPMT");
        lPMTManager->construction();
        lPMTManager->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "_0");
        lHitManager.setNumberOfPMTs(1);
        break;
    }
    case 2:
    {

        lOpticalModule = new mDOM(mData, lPlaceHarness);
        break;
    }
    case 3:
    {

        lOpticalModule = new pDOM(mData, lPlaceHarness);
        break;
    }
    case 4:
    {

        lOpticalModule = new LOM16(mData, lPlaceHarness);
        break;
    }
    case 5:
    {

        lOpticalModule = new LOM18(mData, lPlaceHarness);
        break;
    }
    case 6:
    {
        lOpticalModule = new DEGG(mData, lPlaceHarness);
        break;
    }
    }

    if (lOpticalModule)
    {
        lOpticalModule->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
        lHitManager.setNumberOfPMTs(lOpticalModule->getNumberOfPMTs());
        mOpticalModule = lOpticalModule;
    }

    return mWorldPhysical;
}
