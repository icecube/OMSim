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
    void setSensitiveDetector(G4LogicalVolume* logVol, G4VSensitiveDetector* aSD);
    G4VPhysicalVolume *mWorldPhysical;

protected:
    G4VSolid *mWorldSolid;
    G4LogicalVolume *mWorldLogical;
    virtual void constructWorld() = 0;
    virtual void constructDetector() = 0;
    InputDataManager *mData;
};

#endif
//
