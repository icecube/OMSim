/**
 * @file
 * @brief Defines the OMSimDetectorConstruction class for the radioactive decays simulation.
 * @ingroup radioactive
 */
#ifndef OMSimDetectorConstruction_h
#define OMSimDetectorConstruction_h 1

#include "OMSimMDOM.hh"
#include "abcDetectorComponent.hh"

#include <G4Orb.hh>
#include <G4VUserDetectorConstruction.hh>


/**
 * @class OMSimDetectorConstruction
 * @brief Detector construction of radioactive decays simulation.
 * @ingroup radioactive
 */
class OMSimDetectorConstruction : public G4VUserDetectorConstruction
{
public:
    OMSimDetectorConstruction();
    ~OMSimDetectorConstruction();
    G4VPhysicalVolume *Construct();
    G4VPhysicalVolume *mWorldPhysical;
    OpticalModule* mOpticalModule;

private:
    G4Orb *mWorldSolid;
    G4LogicalVolume *mWorldLogical;
    void constructWorld();
    InputDataManager *mData;
};

#endif
//
