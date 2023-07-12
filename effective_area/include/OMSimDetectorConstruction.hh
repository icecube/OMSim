#ifndef OMSimDetectorConstruction_h
#define OMSimDetectorConstruction_h 1
#include "G4LogicalVolume.hh"
#include "G4Orb.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"
#include "OMSimInputData.hh"
#include "OMSimPMTConstruction.hh"
#include "OMSimMDOM.hh"

class OMSimDetectorConstruction : public G4VUserDetectorConstruction
{
public:
    OMSimDetectorConstruction();
    ~OMSimDetectorConstruction();
    G4VPhysicalVolume *Construct();
    G4VPhysicalVolume *mWorldPhysical;
    mDOM *mMDOM;

private:
    G4Orb *mWorldSolid;
    G4LogicalVolume *mWorldLogical;

    void constructWorld();
    InputDataManager *mData;
};

#endif
//
