#include "OMSimDetectorConstruction.hh"
#include "OMSimPDOM.hh"
#include "OMSimLOM16.hh"
#include "OMSimLOM18.hh"
#include "OMSimDEGG.hh"
#include "OMSimMDOM.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include "OMSimLogger.hh"

#include "G4SDManager.hh"

OMSimDetectorConstruction::OMSimDetectorConstruction()
    : mWorldSolid(0), mWorldLogical(0), mWorldPhysical(0)
{
}

OMSimDetectorConstruction::~OMSimDetectorConstruction()
{
    log_trace("OMSimDetectorConstruction destructor called");
    mSensitiveDetectors.clear();
    if (mData){
        delete mData;
        mData = nullptr;
    }
    
    log_trace("OMSimDetectorConstruction destructor finished");
}

/**
 * @brief Constructs the selected detector from the command line argument and returns the physical world volume.
 * @return Pointer to the physical world volume
 */
G4VPhysicalVolume *OMSimDetectorConstruction::Construct()
{
    log_trace("Starting detector construction");
    mData = new InputDataManager();
    mData->searchFolders();
    constructWorld();
    constructDetector();

    return mWorldPhysical;
}

void OMSimDetectorConstruction::ConstructSDandField()
{
    log_trace("ConstructSDandField started");

    // G4SDManager* lSDManager = G4SDManager::GetSDMpointer();
    for (const auto& sdInfo : mSensitiveDetectors) {
        G4String sdName = sdInfo.sensitiveDetector->GetName();
        // if (!lSDManager->FindSensitiveDetector(sdName)) {
        //     lSDManager->AddNewDetector(sdInfo.sensitiveDetector);
        //     log_trace("Added new sensitive detector: {}", sdName);
        // }
        SetSensitiveDetector(sdInfo.logicalVolume, sdInfo.sensitiveDetector);
        log_trace("Set sensitive detector {} for logical volume {}", 
                  sdName, sdInfo.logicalVolume->GetName());
    }
    log_trace("ConstructSDandField finished");
}

void OMSimDetectorConstruction::registerSensitiveDetector(G4LogicalVolume *pLogVol, G4VSensitiveDetector *pSD)
{
    log_trace("Registering logical volume {} as sensitive detector {}", pLogVol->GetName(), pSD->GetName());
    mSensitiveDetectors.push_back({pLogVol, pSD});
}