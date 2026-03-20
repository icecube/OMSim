#include "OMSimEffectiveAreaDetector.hh"
#include "OMSimPDOM.hh"
#include "OMSimLOM16.hh"
#include "OMSimLOM18.hh"
#include "OMSimDEGG.hh"
#include "OMSimMDOM.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include "OMSimSensitiveDetector.hh"
#include "G4SDManager.hh"
#include "G4NistManager.hh"
#include <G4Orb.hh>
#include "G4Box.hh"
#include "G4Ellipsoid.hh"
#include "G4LogicalVolume.hh"
#include "G4AssemblyVolume.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4Material.hh"





double hc_eVnm = 1239.84193;
/**
 * @brief Constructs the world volume (sphere).
 */
void OMSimEffectiveAreaDetector::constructWorld()
{
    m_worldSolid = new G4Box("World", 370*cm, (370)*cm, 370*cm);
    m_worldLogical = new G4LogicalVolume(m_worldSolid, m_data->getMaterial("Ri_Air"), "World_log", 0, 0, 0);
    m_worldPhysical = new G4PVPlacement (0, G4ThreeVector(0.,0.,0.), m_worldLogical, "World_phys", 0, false, 0);
    m_worldLogical->SetVisAttributes(G4VisAttributes(G4Colour(0.0, 0.0, 0.0, 0.0)));
 
    // water tank
    G4bool checkOverlaps = true;
  //  G4Box* lBoxPool = new G4Box("Pool", 91*cm, (370/2+1)*cm, 91*cm);
  //  G4Box* lWaterSolid = new G4Box("Water", 90*cm, (370/2)*cm, 90*cm);

//######### durschnitllich #####
    G4Box *lBoxPool = new G4Box("Pool", (346.8/2) * cm, (168.6/2+1)* cm, 144/2 * cm);  // Pool
    G4Box *lWaterSolid = new G4Box("Water", 345.8/2 * cm, 167.6/2 * cm, 143/2 * cm);  // Water

//######### max. Länge #####
  //  G4Box *lBoxPool = new G4Box("Pool", (353.5/2) * cm, (174.5/2+1)* cm, 146/2 * cm);  // Pool
  //  G4Box *lWaterSolid = new G4Box("Water", 352.5/2 * cm, 173.5/2 * cm, 145/2 * cm);  // Water

//######### min. Länge #####
   // G4Box *lBoxPool = new G4Box("Pool", (342.2/2) * cm, (163.2/2+1)* cm, 142/2 * cm);  // Pool
   // G4Box *lWaterSolid = new G4Box("Water", 341.2/2 * cm, 162.2/2 * cm, 141/2 * cm);  // Water


    //G4SubtractionSolid *lBoxPool = new G4SubtractionSolid("recipiente", lBoxPool1, mWaterSolid, 0, G4ThreeVector(0, 0, 1 * cm)); // final pool
    G4LogicalVolume *lPoolLogical = new G4LogicalVolume(lBoxPool, m_data->getMaterial("NoOptic_Absorber"), "Pool", 0, 0, 0);
    G4PVPlacement *lPoolPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0 * cm), lPoolLogical, "Pool_phys", m_worldLogical, false, 0, checkOverlaps);
    lPoolLogical->SetVisAttributes(G4VisAttributes(G4Colour(0.8, 0.8, 0.8, 0.2)));

    mWaterLogical = new G4LogicalVolume(lWaterSolid, m_data->getMaterial("Ri_Air"), "Water_log", 0, 0, 0);
    mWaterPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0* cm), mWaterLogical, "Water_phys", lPoolLogical, false, 0, checkOverlaps);
    mWaterLogical->SetVisAttributes(G4VisAttributes(G4Colour(0.0, 0.4, 1.0, 0.2)));

   // G4OpticalSurface *mWater_optical = new G4OpticalSurface("water optical");
    G4LogicalBorderSurface *watersurface = new G4LogicalBorderSurface("Water_skin", mWaterPhysical, lPoolPhysical, m_data->getOpticalSurface("Surf_Refl_tank_plastic"));
 
}

/**
 * @brief Constructs the selected detector from the command line argument and returns the physical world volume.
 * @return Pointer to the physical world volume
 */
void OMSimEffectiveAreaDetector::constructDetector()
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

    }

    if (opticalModule)
    {
        log_critical("Here");
        opticalModule->placeIt(G4ThreeVector(140*cm, 0*cm, 0*cm), G4RotationMatrix(), mWaterLogical, ""); // right position coordinates needed
        opticalModule->configureSensitiveVolume(this);
    }
}
