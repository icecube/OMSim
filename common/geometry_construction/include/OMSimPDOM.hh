/**
 * @file OMSimPDOM.hh
 * @brief Implementation of the pDOM (deepcore) or Gen1 DOM..
 * @ingroup common
 */
#pragma once

#include "OMSimPMTConstruction.hh"
#include "OMSimOpticalModule.hh"
class DOMHarness;

/**
 * @class DOM
 * @brief DOM class represents the construction of pDOM (deepcore) or Gen1 DOM.
 * @ingroup common
 */
class DOM : public OMSimOpticalModule
{
public:
    DOM(G4bool p_placeHarness = true, G4bool p_deepcore = false);
    ~DOM(){};
    void construction();
    DOMHarness *m_harness;
    G4bool m_placeHarness = true;
    G4String getName()
    {
        std::stringstream ss;
        ss << "/DOM/" << m_index;
        return ss.str();
    }
    double getPressureVesselWeight() {return 9.07*kg;};
    int getNumberOfPMTs() { return 1;};
};

