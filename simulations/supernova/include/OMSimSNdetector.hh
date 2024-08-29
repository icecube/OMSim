#pragma once
#include "OMSimDetectorConstruction.hh"


/**
 * @class OMSimSNdetector
 * @brief Class for detector construction in the SN neutrino simulation
 * @ingroup sngroup
 */
class OMSimSNdetector : public OMSimDetectorConstruction
{
    public: 
    OMSimSNdetector(){};
    ~OMSimSNdetector(){};

private:
    /**
     * @brief Constructs the world volume (cylinder).
     */
    void constructWorld();
    void constructDetector();
};

