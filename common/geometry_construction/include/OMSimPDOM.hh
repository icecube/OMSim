/**
 * @file OMSimPDOM.hh
 * @brief Implementation of the pDOM/Gen1 DOM class.
 * @ingroup common
 */
#pragma once

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
    G4bool m_placeHarness;
    G4String getName()
    {
        std::stringstream ss;
        ss << "/pDOM/" << m_index;
        return ss.str();
    }
    double getPressureVesselWeight() {return 9.07*kg;};
    int getNumberOfPMTs() { return 1;};
};

