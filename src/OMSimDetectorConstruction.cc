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


OMSimDetectorConstruction::OMSimDetectorConstruction()
    : mWorldSolid(0), mWorldLogical(0), mWorldPhysical(0)
{
}

OMSimDetectorConstruction::~OMSimDetectorConstruction()
{
}

/**
 * Construct the world volume
 */
void OMSimDetectorConstruction::ConstructWorld()
{
    mWorldSolid = new G4Orb("World", OMSimCommandArgsTable::getInstance().get<G4double>("world_radius") * m);
    mWorldLogical = new G4LogicalVolume(mWorldSolid, mData->GetMaterial("argWorld"), "World_log", 0, 0, 0);
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

    mData = new OMSimInputData();
    mData->SearchFolders();

    ConstructWorld();

    
    
    G4Box* Photocathode = new G4Box("Photocathode", 20*cm, 20*cm, 10*nm);
    G4Box* Glass = new G4Box("Glass", 20*cm, 20*cm, 1*mm);
    G4Box* Vacuum = new G4Box("Vacuum", 20*cm, 20*cm, 1*cm);

    
    G4LogicalVolume* PhotocathodeLog = new G4LogicalVolume(Photocathode, mData->GetMaterial("RiAbs_Photocathode"), "Photocathode");
    G4LogicalVolume* GlassLog = new G4LogicalVolume(Glass, mData->GetMaterial("RiAbs_Glass_Tube"), "PMTGlass");
    G4LogicalVolume* VacuumLog = new G4LogicalVolume(Vacuum, mData->GetMaterial("Ri_Vacuum"), "PMTvacuum");

    
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), GlassLog, "PMTGlass", mWorldLogical, false, 0, true);
    //new G4PVPlacement(0, G4ThreeVector(0, 0, -(1*mm+10*nm)), PhotocathodeLog, "Photocathode", mWorldLogical, false, 0, true);
    new G4PVPlacement(0, G4ThreeVector(0, 0, -(1*cm+1*mm)), VacuumLog, "PMTvacuum", mWorldLogical, false, 0, true);

    /*    
    switch (OMSimCommandArgsTable::getInstance().get<G4int>("detector_type")) {
        case 0: {
            critical("Custom detector not implemented yet!");
            break;
        }
        case 1: {
            info("Constructing single PMT");
            mPMTManager = new OMSimPMTConstruction(mData);
            if (gVisual) {
                mPMTManager->SelectPMT("pmt_Hamamatsu_R15458_20nm");
            } else {
                mPMTManager->SelectPMT("pmt_Hamamatsu_R15458_20nm");
            }
            mPMTManager->SimulateHACoating();
            mPMTManager->SimulateInternalReflections();
            mPMTManager->Construction();
            mPMTManager->PlaceIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "_0");
            break;
        }
        case 2: {
            info("Constructing mDOM");
            mDOM *lOpticalModule = new mDOM(mData);
            lOpticalModule->PlaceIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
            break;
        }
        case 3: {
            info("Constructing PDOM");
            pDOM *lOpticalModule = new pDOM(mData);
            lOpticalModule->PlaceIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
            break;
        }
        case 4: {
            info("Constructing LOM16");
            LOM16 *lOpticalModule = new LOM16(mData);
            lOpticalModule->PlaceIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
            break;
        }
        case 5: {
            info("Constructing LOM18");
            LOM18 *lOpticalModule = new LOM18(mData);
            lOpticalModule->PlaceIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
            break;
        }
        case 6: {
            info("Constructing DEGG");
            DEgg *lOpticalModule = new DEgg(mData);
            lOpticalModule->PlaceIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
            break;
        }
    }
    
    */
    
    return mWorldPhysical;
}
