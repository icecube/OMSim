/**
 * @file OMSimPDOM.hh
 * @brief Implementation of the pDOM/Gen1 DOM class.
 * @ingroup common
 */
#ifndef OMSimPDOM_h
#define OMSimPDOM_h 1

#include "OMSimPMTConstruction.hh"
#include "OMSimOpticalModule.hh"

/**
 * @class pDOM
 * @brief pDOM class represents the construction of pDOM/Gen1 DOM.
 * @ingroup common
 */
class pDOM : public OMSimOpticalModule
{
public:
    pDOM(G4bool pPlaceHarness = true);
    ~pDOM(){};
    void construction();
    G4bool mPlaceHarness;
    G4String getName()
    {
        std::stringstream ss;
        ss << "/pDOM/" << mIndex;
        return ss.str();
    }
    double getPressureVesselWeight() {return 9.07*kg;};
    int getNumberOfPMTs() { return 1;};
};

#endif