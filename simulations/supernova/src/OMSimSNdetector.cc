#include "OMSimSNdetector.hh"
#include "OMSimMDOM.hh"
#include "OMSimPDOM.hh"
#include "OMSimLOM16.hh"
#include "OMSimLOM18.hh"
#include "OMSimDEGG.hh"
#include "OMSimHitManager.hh"
#include "G4Navigator.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"

#include <G4Tubs.hh>
/**
 * Construct the world volume
 */
void OMSimSNdetector::constructWorld(){
    mWorldSolid = new G4Tubs("World",
                 0.*cm, 
                 OMSimCommandArgsTable::getInstance().get<G4double>("wradius") * m,
                 OMSimCommandArgsTable::getInstance().get<G4double>("wheight") * m,
                 0.*deg, 
                 360.*deg); 
  
    mWorldLogical = new G4LogicalVolume(mWorldSolid, mData->getMaterial("argWorld"), "World_log", 0, 0, 0);
    mWorldPhysical = new G4PVPlacement (0, G4ThreeVector(0.,0.,0.), mWorldLogical, "World_phys", 0, false, 0);
    G4VisAttributes* World_vis= new G4VisAttributes(G4Colour(0.45,0.5,0.35,0.2));
    mWorldLogical->SetVisAttributes(World_vis);
}

void OMSimSNdetector::constructDetector() {
    
    OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

    bool lPlaceHarness = OMSimCommandArgsTable::getInstance().get<bool>("place_harness");

    OMSimOpticalModule *lOpticalModule = nullptr;

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
        OMSimPMTConstruction *lPMTManager = new OMSimPMTConstruction();
        lPMTManager->selectPMT("argPMT");
        lPMTManager->construction();
        lPMTManager->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "_0");
        lHitManager.setNumberOfPMTs(1, 0);
        lPMTManager->configureSensitiveVolume(this, "/PMT/0");
        break;
    }
    case 2:
    {

        lOpticalModule = new mDOM(lPlaceHarness);
        break;
    }
    case 3:
    {

        lOpticalModule = new pDOM(lPlaceHarness);
        break;
    }
    case 4:
    {

        lOpticalModule = new LOM16(lPlaceHarness);
        break;
    }
    case 5:
    {

        lOpticalModule = new LOM18(lPlaceHarness);
        break;
    }
    case 6:
    {
        lOpticalModule = new DEGG(lPlaceHarness);
        break;
    }
    }

    if (lOpticalModule)
    {
        lOpticalModule->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
        lOpticalModule->configureSensitiveVolume(this);
    }
}