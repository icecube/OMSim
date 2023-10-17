/**
 * @file OpticalModule.h
 * @brief Defines the OpticalModule interface for optical modules.
 * @details
 * This header file contains the declaration of the OpticalModule class which serves as the base class/interface
 * for optical modules (OMs) in the detector. The OpticalModule class encapsulates common attributes and operations
 * that are applicable to OMs, such as retrieving the weight of the pressure vessel and the number of PMTs in the OM.
 * It also maintains a reference to a PMT manager, which is responsible for PMT-related functionalities.
 * @ingroup common
 */
#ifndef OpticalModule_h
#define OpticalModule_h 1
#include "abcDetectorComponent.hh"
//#include "OMSimDetectorConstruction.hh"
#include "OMSimPMTConstruction.hh"
class OMSimDetectorConstruction;


/**
 *  @class OpticalModule
 *  @brief Base class for OMs works as interface
 *  @ingroup common
 */
class OpticalModule : public abcDetectorComponent
{
public:
    /**
     *  @brief Virtual method to get the weight of the pressure vessel.
     *  @details This method should be overridden in derived classes to provide the weight of the pressure vessel for the
     *           specific optical module.
     *  @return Weight of the pressure vessel.
     */
    virtual double getPressureVesselWeight() = 0;

    /**
     *  @brief Virtual method to get the number of PMTs in the optical module.
     *  @details This method should be overridden in derived classes to provide the number of PMTs in the specific optical
     *           module.
     *  @return Number of PMTs in the optical module.
     */
    virtual int getNumberOfPMTs() = 0;

    void configureSensitiveVolume(OMSimDetectorConstruction *pDetConst){mPMTManager->configureSensitiveVolume(pDetConst, getName());};

    virtual G4String getName() = 0;

    OMSimPMTConstruction *getPMTmanager() { return mPMTManager; };

    virtual ~OpticalModule()
    {
        if (mPMTManager != nullptr)
        {
            delete mPMTManager;
            mPMTManager = nullptr;
        }
    }

    G4int mIndex;

protected:
    OMSimPMTConstruction *mPMTManager;
};

#endif
