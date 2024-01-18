#include "OMSimEffectiveAreaDetector.hh"
#include "OMSimPDOM.hh"
#include "OMSimLOM16.hh"
#include "OMSimLOM18.hh"
#include "OMSimDEGG.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"

#include "OMSimMDOM.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimSensitiveDetector.hh"
#include "OMSimHitManager.hh"
#include "G4SDManager.hh"
#include <G4Orb.hh>
#include "G4LogicalBorderSurface.hh"


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


/*
    mDOM* mOpticalModule = new mDOM(mData, lPlaceHarness);
    mOpticalModule->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
    mOpticalModule->configureSensitiveVolume(this);

    lHitManager.setNumberOfPMTs(mOpticalModule->getNumberOfPMTs(), mOpticalModule->mIndex);

    mDOM* mOpticalModule2 = new mDOM(mData, lPlaceHarness, 1);
    mOpticalModule2->placeIt(G4ThreeVector(0, 0.5*m, -1.5*m), G4RotationMatrix(), mWorldLogical, "");
    mOpticalModule2->configureSensitiveVolume(this);
    lHitManager.setNumberOfPMTs(mOpticalModule2->getNumberOfPMTs(), mOpticalModule2->mIndex);
*/

    G4double lPhotocathodeThickness = 25*nm;
    G4double lGlassThickness = 1*cm;
    G4double lVacThinkness = 5*cm;
    G4double AbsorberEdgeLength = 5*m;

    //half the length to place the Volumes later
    G4double halfPhotocathodeThickness = lPhotocathodeThickness / 2.0;
    G4double halfGlassThickness = lGlassThickness / 2.0;
    G4double halfVacuumThickness = lVacThinkness / 2.0;

    G4Box* Photocathode = new G4Box("Photocathode", 20*cm, 20*cm, lPhotocathodeThickness);
    G4Box* Glass = new G4Box("Glass", 20*cm, 20*cm, lGlassThickness);
    G4Box* Vacuum = new G4Box("Vacuum", 20*cm, 20*cm, lVacThinkness);
    
    G4Box* TopAbsorber = new G4Box("TopAbsorber", AbsorberEdgeLength, AbsorberEdgeLength, 20*nm);
    G4Box* BottomAbsorber = new G4Box("BottomAbsorber", AbsorberEdgeLength, AbsorberEdgeLength, 20*nm);

/* TopAbsorber, BottomAbsorber ->  two sensitive objects (photocathodes) on top and under the vacuum
 * to count reflected and transmitted photons 
 */ 
 
    G4LogicalVolume* PhotocathodeLog = new G4LogicalVolume(Photocathode, mData->getMaterial("RiAbs_NicoPalladium_"), "Photocathode");
    G4LogicalVolume* GlassLog = new G4LogicalVolume(Glass, mData->getMaterial("RiAbs_NicoGlass_"), "PMTGlass");
    G4LogicalVolume* VacuumLog = new G4LogicalVolume(Vacuum, mData->getMaterial("Ri_Vacuum"), "PMTvacuum");
    
    G4LogicalVolume* TopAbsorberLog = new G4LogicalVolume(TopAbsorber, mData->getMaterial("RiAbs_TotalAbsorber"), "TopAbsorber");
    G4LogicalVolume* BottomAbsorberLog = new G4LogicalVolume(BottomAbsorber, mData->getMaterial("RiAbs_TotalAbsorber"), "BottomAbsorber");

    const G4VisAttributes *lGlassVis = new G4VisAttributes(G4Colour(0.7, 0.7, 0.8, 0.25));
    const G4VisAttributes *lVacuumColor = new G4VisAttributes(G4Colour(0, 1, 1, 0.25));
    const G4VisAttributes *lAbsorberVis = new G4VisAttributes(G4Colour(0.0, 0.0, 0.8, 0.05));
    GlassLog->SetVisAttributes(lGlassVis);
    VacuumLog->SetVisAttributes(lVacuumColor);
    
    TopAbsorberLog->SetVisAttributes(lAbsorberVis);
    BottomAbsorberLog->SetVisAttributes(lAbsorberVis);
    
    OMSimSensitiveDetector* lSensitiveDetector = new OMSimSensitiveDetector("/Phot/0", DetectorType::BoundaryPhotonDetector);
    lSensitiveDetector->setPMTResponse(&NoResponse::getInstance());
    setSensitiveDetector(GlassLog, lSensitiveDetector);
    lHitManager.setNumberOfPMTs(1, 0);
    
     OMSimSensitiveDetector* lSensitiveDetector1 = new OMSimSensitiveDetector("/TopAbs/1", DetectorType::VolumePhotonDetector);
    lSensitiveDetector->setPMTResponse(&NoResponse::getInstance());
    setSensitiveDetector(TopAbsorberLog, lSensitiveDetector1);
    lHitManager.setNumberOfPMTs(1, 1);

     OMSimSensitiveDetector* lSensitiveDetector2 = new OMSimSensitiveDetector("/BotAbs/2", DetectorType::VolumePhotonDetector);
    lSensitiveDetector->setPMTResponse(&NoResponse::getInstance());
    setSensitiveDetector(BottomAbsorberLog, lSensitiveDetector2);
    lHitManager.setNumberOfPMTs(1, 2);

    G4OpticalSurface* NicoSurface = new G4OpticalSurface("NicoSurface");
    NicoSurface->SetType(coated);


    // Place the glass
    G4PVPlacement* GlassPhys = new G4PVPlacement(
        0, 
        G4ThreeVector(0, 0, 0), // center of the world volume
        GlassLog, 
        "PMTGlass", 
        mWorldLogical, 
        false, 
        0, 
        false);

    // Place the photocathode just below the glass
    G4PVPlacement* PhotocathodePhys = new G4PVPlacement(
        0, 
        G4ThreeVector(0, 0, -halfGlassThickness - halfPhotocathodeThickness), 
        PhotocathodeLog, 
        "Photocathode", 
        mWorldLogical, 
        false, 
        0, 
        false);

    // Place the vacuum below the photocathode
    G4PVPlacement* VacuumPhys = new G4PVPlacement(
        0, 
        G4ThreeVector(0, 0, -halfGlassThickness - lPhotocathodeThickness - halfVacuumThickness), 
        VacuumLog, 
        "PMTvacuum", 
        mWorldLogical, 
        false, 
        0, 
        false);


    //new G4LogicalBorderSurface("PMT_mirrorglass", GlassPhys, VacuumPhys, NicoSurface);
    //new G4LogicalBorderSurface("PMT_mirrorglass", VacuumPhys, GlassPhys, NicoSurface);

    new G4PVPlacement(0, G4ThreeVector(0, 0, 30*cm), TopAbsorberLog, "TopAbsorber", mWorldLogical, false, 0, true);
    new G4PVPlacement(0, G4ThreeVector(0, 0, -(3*lVacThinkness + lPhotocathodeThickness)), BottomAbsorberLog, "BottomAbsorber", mWorldLogical, false, 0, true);

 
}