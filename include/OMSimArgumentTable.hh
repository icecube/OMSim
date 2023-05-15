#ifndef OMSimArgumentTable_HH
#define OMSimArgumentTable_HH

#include "globals.hh"

class OMSimArgumentTable {
public:
    static OMSimArgumentTable& getInstance()
    {
        static OMSimArgumentTable instance; // Guaranteed to be destroyed.
                                // Instantiated on first use.
        return instance;
    }

    OMSimArgumentTable(OMSimArgumentTable const&) = delete; // Don't allow copy
    void operator=(OMSimArgumentTable const&) = delete; // Don't allow assignment

    G4double worldSize;
    G4int simEvents;
    G4String gunFilename;
    G4double distance;
    G4double beamDiam;
    G4double theta;
    G4double phi;
    G4double waveLen;
    G4String hitsFilename;
    G4int hitType;
    G4int pmt;
    G4int environment;
    G4bool visual;
    G4bool CADImport;
    G4bool placeHarness;
    G4bool interactive;
    G4bool noHeader;

    G4int glass;
    G4int gel;
    G4double refConeAngle;
    G4int coneMat;
    G4int holderColor;
    G4int dom;
    G4int harness;
    G4int ropeNumber;

private:
    OMSimArgumentTable() {} // Make constructor private so can't be called directly
};

#endif // CONFIG_HH
