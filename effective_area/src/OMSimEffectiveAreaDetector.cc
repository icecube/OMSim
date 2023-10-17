#include "OMSimEffectiveAreaDetector.hh"
#include "OMSimPDOM.hh"
#include "OMSimLOM16.hh"
#include "OMSimLOM18.hh"
#include "OMSimDEGG.hh"
#include "OMSimMDOM.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include "G4SDManager.hh"
#include <G4Orb.hh>


/**
 * @brief Constructs the world volume (sphere).
 */
void OMSimEffectiveAreaDetector::constructWorld()
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
void OMSimEffectiveAreaDetector::constructDetector()
{
    OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

    bool lPlaceHarness = OMSimCommandArgsTable::getInstance().get<bool>("place_harness");
    mDOM* mOpticalModule = new mDOM(mData, lPlaceHarness);
    mOpticalModule->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
    mOpticalModule->configureSensitiveVolume(this);

    lHitManager.setNumberOfPMTs(mOpticalModule->getNumberOfPMTs(), mOpticalModule->mIndex);

    mDOM* mOpticalModule2 = new mDOM(mData, lPlaceHarness, 1);
    mOpticalModule2->placeIt(G4ThreeVector(0, 0, -1.5*m), G4RotationMatrix(), mWorldLogical, "");
    mOpticalModule2->configureSensitiveVolume(this);
    lHitManager.setNumberOfPMTs(mOpticalModule2->getNumberOfPMTs(), mOpticalModule2->mIndex);

}
