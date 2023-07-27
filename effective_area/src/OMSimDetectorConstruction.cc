/** @file OMSimDetectorConstruction.cc
 *  @brief User defined detector.
 *
 * You should define and construct the detector here...this template is an example for a single arg module. 
 *
 *  @author Martin Unland - modified by Markus Dittmer
 *  @date October 2021 - modified at February 2022
 * 
 *  @version Geant4 10.7
 *  
 *  @todo 
 */

#include "OMSimDetectorConstruction.hh"

#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Transform3D.hh"

#include "OMSimInputData.hh"

#include "OMSimPMTConstruction.hh"
#include "OMSimMDOM.hh"
#include "OMSimPDOM.hh"
#include "OMSimLOM16.hh"
#include "OMSimLOM18.hh"
#include "OMSimDEGG.hh"
#include "OMSimLogger.hh" 

#include "G4Box.hh"
#include "G4LogicalVolume.hh"

#include "OMSimCommandArgsTable.hh"
#include "OMSimMDOMFlasher.hh"


OMSimDetectorConstruction::OMSimDetectorConstruction()
    : mWorldSolid(0), mWorldLogical(0), mWorldPhysical(0)
{
}

OMSimDetectorConstruction::~OMSimDetectorConstruction()
{
    delete mData;
}

/**
 * Construct the world volume
 */
void OMSimDetectorConstruction::constructWorld()
{
    mWorldSolid = new G4Orb("World", OMSimCommandArgsTable::getInstance().get<G4double>("world_radius") * m);
    mWorldLogical = new G4LogicalVolume(mWorldSolid, mData->getMaterial("argWorld"), "World_log", 0, 0, 0);
    mWorldPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), mWorldLogical, "World_phys", 0, false, 0);
    G4VisAttributes* World_vis= new G4VisAttributes(G4Colour(0.45,0.5,0.35,0.));
    mWorldLogical->SetVisAttributes(World_vis);
}

/**
 * Construct all solids needed for your study. Call OMSimOMConstruction for simulations with optical modules
 * and OMSimPMTConstruction for simulations with only PMTs.
 * @return World physical for the main
 */
G4VPhysicalVolume *OMSimDetectorConstruction::Construct()
{

    mData = new InputDataManager();
    mData->searchFolders();

    constructWorld();

    bool lPlaceHarness = OMSimCommandArgsTable::getInstance().get<bool>("place_harness");

    switch (OMSimCommandArgsTable::getInstance().get<G4int>("detector_type")) {
        case 0: {
            log_critical("Custom detector not implemented yet!");
            break;
        }
        case 1: {
            log_info("Constructing single PMT");
            OMSimPMTConstruction* lPMTManager = new OMSimPMTConstruction(mData);
            lPMTManager->SelectPMT("argPMT");
            lPMTManager->construction();
            lPMTManager->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "_0");
            break;
        }
        case 2: {
            log_info("Constructing mDOM");
            mDOM* mMDOM = new mDOM(mData, lPlaceHarness);
            mMDOM->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
            break;
        }
        case 3: {
            log_info("Constructing PDOM");
            pDOM *lOpticalModule = new pDOM(mData, lPlaceHarness);
            lOpticalModule->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
            break;
        }
        case 4: {
            log_info("Constructing LOM16");
            LOM16 *lOpticalModule = new LOM16(mData, lPlaceHarness);
            lOpticalModule->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
            break;
        }
        case 5: {
            log_info("Constructing LOM18");
            LOM18 *lOpticalModule = new LOM18(mData, lPlaceHarness);
            lOpticalModule->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
            break;
        }
        case 6: {
            log_info("Constructing DEGG");
            DEGG *lOpticalModule = new DEGG(mData, lPlaceHarness);
            lOpticalModule->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
            break;
        }
    }
    
    
    
    return mWorldPhysical;
}
