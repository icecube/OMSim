/**
 * @file OMSimDetectorConstruction.h
 * @brief Defines the OMSimDetectorConstruction class for effective area simulation detector construction.
 * @ingroup EffectiveArea
 */

#ifndef OMSimDetectorConstruction_h
#define OMSimDetectorConstruction_h 1

#include "OMSimMDOM.hh"

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
    G4VPhysicalVolume *mWorldPhysical;

private:
    G4Orb *mWorldSolid;
    G4LogicalVolume *mWorldLogical;

    void constructWorld();
    InputDataManager *mData;
};

#endif
//
