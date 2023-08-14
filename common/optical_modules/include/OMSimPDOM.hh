#ifndef OMSimPDOM_h
#define OMSimPDOM_h 1

#include "OMSimPMTConstruction.hh"
#include "OMSimOpticalModule.hh"

/**
 * @class pDOM
 * @brief pDOM class represents the construction of pDOM/Gen1 DOM.
 * @ingroup common
 */
class pDOM : public OpticalModule
{
public:
    pDOM(InputDataManager *pData, G4bool pPlaceHarness = true);
    ~pDOM();
    void construction();
    G4bool mPlaceHarness;

    double getPressureVesselWeight() {return 9.07;};
    int getNumberOfPMTs() { return 1;};
};

#endif