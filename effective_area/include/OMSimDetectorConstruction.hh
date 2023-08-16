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

    /**
     * @brief Constructs the selected detector from the command line argument and returns the physical world volume.
     * @return Pointer to the physical world volume
     */
    G4VPhysicalVolume *Construct();
    G4VPhysicalVolume *mWorldPhysical;

private:
    G4Orb *mWorldSolid;
    G4LogicalVolume *mWorldLogical;
    /**
     * @brief Constructs the world volume (sphere).
     */
    void constructWorld();
    InputDataManager *mData;
};

#endif
//
