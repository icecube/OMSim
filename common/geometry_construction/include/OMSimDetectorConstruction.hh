/**
 * @file OMSimDetectorConstruction.h
 * @brief Defines the OMSimDetectorConstruction class for effective area simulation detector construction.
 * @ingroup EffectiveArea
 */

#pragma once
#include "OMSimOpticalModule.hh"

#include <G4Orb.hh>
#include <G4VUserDetectorConstruction.hh>

/**
 * @class OMSimDetectorConstruction
 * @brief Class for detector construction in the effective area simulation.
 * @ingroup EffectiveArea
 */
class OMSimDetectorConstruction : public G4VUserDetectorConstruction
{
public:
    OMSimDetectorConstruction();
    ~OMSimDetectorConstruction();
    G4VPhysicalVolume *Construct();
    void ConstructSDandField() override;
    void registerSensitiveDetector(G4LogicalVolume* logVol, G4VSensitiveDetector* aSD);

    G4VPhysicalVolume *m_worldPhysical;

protected:
    virtual void constructWorld() = 0;
    virtual void constructDetector() = 0;

    G4VSolid *m_worldSolid;
    G4LogicalVolume *m_worldLogical;
    OMSimInputData *m_data;
    struct SDInfo {
        G4LogicalVolume* logicalVolume;
        G4VSensitiveDetector* sensitiveDetector;
    };
    std::vector<SDInfo> m_sensitiveDetectors;
};
