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
    : m_worldSolid(0), m_worldLogical(0), m_worldPhysical(0)
{
    OMSimInputData::init();
    m_data = &OMSimInputData::getInstance();
}

OMSimDetectorConstruction::~OMSimDetectorConstruction()
{
    log_trace("Clearing sensitive detectors");
    m_sensitiveDetectors.clear();

    log_trace("Deleting OMSimInputData");
    OMSimInputData::shutdown();
}

/**
 * @brief Constructs the selected detector from the command line argument and returns the physical world volume.
 * @return Pointer to the physical world volume
 */
G4VPhysicalVolume *OMSimDetectorConstruction::Construct()
{
    log_trace("Starting detector construction");
    constructWorld();
    constructDetector();
    return m_worldPhysical;
}

void OMSimDetectorConstruction::ConstructSDandField()
{
    log_trace("ConstructSDandField started");

    for (const auto& sdInfo : m_sensitiveDetectors) {
        G4String sdName = sdInfo.sensitiveDetector->GetName();
        SetSensitiveDetector(sdInfo.logicalVolume, sdInfo.sensitiveDetector);
        log_trace("Set sensitive detector {} for logical volume {}", 
                  sdName, sdInfo.logicalVolume->GetName());
    }
    log_trace("ConstructSDandField finished");
}

void OMSimDetectorConstruction::registerSensitiveDetector(G4LogicalVolume *pLogVol, G4VSensitiveDetector *pSD)
{
    log_trace("Registering logical volume {} as sensitive detector {}", pLogVol->GetName(), pSD->GetName());
    m_sensitiveDetectors.push_back({pLogVol, pSD});
}