/**
 * @file
 * @brief Defines the OMSimRadDecaysDetector class for the radioactive decays simulation.
 * @ingroup radioactive
 */
#ifndef OMSimRadDecaysDetector_h
#define OMSimRadDecaysDetector_h 1

#include "OMSimOpticalModule.hh"
#include "abcDetectorComponent.hh"
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
    OMSimOpticalModule *mOpticalModule;
private:
    void constructWorld();
    void constructDetector();
    
};

#endif
//
