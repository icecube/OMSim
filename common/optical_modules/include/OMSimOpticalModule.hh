
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
        virtual double get_pressure_vessel_weight() = 0;
        virtual int get_number_of_PMTs() = 0;

        OMSimPMTConstruction* get_PMT_manager() {return mPMTManager;};
        
    protected:
        OMSimPMTConstruction *mPMTManager;

};

#endif
