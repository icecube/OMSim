
#ifndef OpticalModule_h
#define OpticalModule_h 1
#include "abcDetectorComponent.hh"
#include "OMSimPMTConstruction.hh"

/**
 *  @class OpticalModule
 *  @brief Base class for OMs works as interface
 *  @ingroup common
 */
class OpticalModule: public abcDetectorComponent
{
    public:
     /**
     *  @brief Returns weight of pressure vessel in kg.
     */
        virtual double getPressureVesselWeight() = 0;
        virtual int getNumberOfPMTs() = 0;

        OMSimPMTConstruction* getPMTmanager() {return mPMTManager;};
        
    protected:
        OMSimPMTConstruction *mPMTManager;

};

#endif
