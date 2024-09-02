/**
 * @file OMSimOpticalModule.h
 * @brief Defines the OMSimOpticalModule interface for optical modules.
 * @details
 * This header file contains the declaration of the OMSimOpticalModule class which serves as the base class/interface
 * for optical modules (OMs) in the detector. The OMSimOpticalModule class encapsulates common attributes and operations
 * that are applicable to OMs, such as retrieving the weight of the pressure vessel and the number of PMTs in the OM.
 * It also maintains a reference to a PMT manager, which is responsible for PMT-related functionalities.
 * @ingroup common
 */
#pragma once
#include "OMSimDetectorComponent.hh"
#include "OMSimHitManager.hh"
#include "OMSimPMTConstruction.hh"
class OMSimDetectorConstruction;

/**
 *  @class OMSimOpticalModule
 *  @brief Base class for OMs works as interface
 *  @ingroup common
 */
class OMSimOpticalModule : public OMSimDetectorComponent
{
public:
    OMSimOpticalModule(OMSimPMTConstruction* pPMTManager);
    virtual ~OMSimOpticalModule();

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

    void configureSensitiveVolume(OMSimDetectorConstruction *pDetConst);

    virtual G4String getName() = 0;

    OMSimPMTConstruction *getPMTmanager();
    G4int m_index;

protected:
    OMSimPMTConstruction *m_managerPMT;
};

