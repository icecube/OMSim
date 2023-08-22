#ifndef OMSimDetectorConstruction_h
#define OMSimDetectorConstruction_h 1

#include "OMSimMDOM.hh"

#include <G4Tubs.hh>
#include <G4VUserDetectorConstruction.hh>

/**
 * @class OMSimDetectorConstruction
 * @brief Class for detector construction in the SN neutrino simulation
 * @ingroup SN
 */
class OMSimDetectorConstruction : public G4VUserDetectorConstruction
{
    public: OMSimDetectorConstruction();
    ~OMSimDetectorConstruction();
    G4VPhysicalVolume* mWorldPhysical;

    /**
     * @brief Constructs the selected detector from the command line argument and returns the physical world volume.
     * @return Pointer to the physical world volume
     */
    G4VPhysicalVolume* Construct();


private:
    G4Tubs*			mWorldSolid;
    G4LogicalVolume*		mWorldLogical;
    InputDataManager *mData;
    
    /**
     * @brief Constructs the world volume (cylinder).
     */
    void constructWorld();
};


#endif
//
