/**
 * @file OMSimDetectorConstruction.h
 * @brief Defines the OMSimDetectorConstruction class for effective area simulation detector construction.
 * @ingroup EffectiveArea
 */

#ifndef OMSimDetectorConstruction_h
#define OMSimDetectorConstruction_h 1

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
    G4VPhysicalVolume *mWorldPhysical;
    OMSimInputData* getDataManager() {return mData;};

protected:
    G4VSolid *mWorldSolid;
    G4LogicalVolume *mWorldLogical;
    virtual void constructWorld() = 0;
    virtual void constructDetector() = 0;
    OMSimInputData *mData;

    struct SDInfo {
        G4LogicalVolume* logicalVolume;
        G4VSensitiveDetector* sensitiveDetector;
    };
    std::vector<SDInfo> mSensitiveDetectors;
};

#endif
//
