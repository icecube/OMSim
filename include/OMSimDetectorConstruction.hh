#ifndef OMSimDetectorConstruction_h
#define OMSimDetectorConstruction_h 1
#include "G4LogicalVolume.hh"
#include "G4Orb.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"
#include "OMSimInputData.hh"
#include "OMSimPMTConstruction.hh"

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
    
    void ConstructWorld();
    OMSimInputData *mData;
    OMSimPMTConstruction* mPMTManager;
};

#endif
//
