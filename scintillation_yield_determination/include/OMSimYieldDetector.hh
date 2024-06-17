#ifndef OMSimYieldDetector
#define OMSimYieldDetector 1

#include "OMSimOpticalModule.hh"
#include "abcDetectorComponent.hh"
#include "OMSimDetectorConstruction.hh"

#include <G4Orb.hh>



/**
 * @class OMSimRadDecaysDetector
 * @brief Detector construction of radioactive decays simulation.
 * @ingroup radioactive
 */
class OMSimYieldDetector : public OMSimYieldDetector
{
public:
    OMSimYieldDetector(){};
    ~OMSimYieldDetector(){};
    OMSimOpticalModule *mOpticalModule;
    abcDetectorComponent *mSource = nullptr;
private:
    void constructWorld();
    void constructDetector();
    
};

#endif
//
