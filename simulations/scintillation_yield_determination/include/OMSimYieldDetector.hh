#pragma once
#include "OMSimOpticalModule.hh"
#include "OMSimDetectorComponent.hh"
#include "OMSimDetectorConstruction.hh"

#include <G4Orb.hh>



/**
 * @class OMSimRadDecaysDetector
 * @brief Detector construction of radioactive decays simulation.
 * @ingroup radioactive
 */
class OMSimYieldDetector : public OMSimDetectorConstruction
{
public:
    OMSimYieldDetector(){};
    ~OMSimYieldDetector(){};
    OMSimOpticalModule *mOpticalModule = nullptr;
    OMSimDetectorComponent *m_source = nullptr;
private:
    void constructWorld();
    void constructDetector();
    
};
