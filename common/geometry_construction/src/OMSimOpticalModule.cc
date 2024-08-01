#include "OMSimOpticalModule.hh"
#include "OMSimHitManager.hh"
#include "OMSimPMTConstruction.hh"

OMSimOpticalModule::OMSimOpticalModule(OMSimPMTConstruction *pPMTManager) : abcDetectorComponent(), mPMTManager(pPMTManager)
{
    log_trace("Constructor of OMSimOpticalModule");
}

OMSimOpticalModule::~OMSimOpticalModule()
{
    if (mPMTManager != nullptr)
    {
        delete mPMTManager;
        mPMTManager = nullptr;
    }
}

OMSimPMTConstruction *OMSimOpticalModule::getPMTmanager()
{
    log_trace("Getting PMT instance used in optical module instance");
    return mPMTManager;
}

void OMSimOpticalModule::configureSensitiveVolume(OMSimDetectorConstruction *pDetConst)
{   
    mIndex = OMSimHitManager::getInstance().getNextDetectorIndex();
    log_debug("Configuring {} as sensitive detector", getName());
    OMSimHitManager::getInstance().setNumberOfPMTs(getNumberOfPMTs(), mIndex);
    mPMTManager->configureSensitiveVolume(pDetConst, getName());
}