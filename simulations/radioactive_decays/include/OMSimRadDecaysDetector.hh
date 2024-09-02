/**
 * @file
 * @brief Defines the OMSimRadDecaysDetector class for the radioactive decays simulation.
 * @ingroup radioactive
 */
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
class OMSimRadDecaysDetector : public OMSimDetectorConstruction
{
public:
    OMSimRadDecaysDetector(){};
    ~OMSimRadDecaysDetector(){};
    OMSimOpticalModule *m_opticalModule;
private:
    void constructWorld();
    void constructDetector();
    
};

