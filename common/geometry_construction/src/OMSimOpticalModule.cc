#include "OMSimOpticalModule.hh"
#include "OMSimHitManager.hh"
#include "OMSimPMTConstruction.hh"

OMSimOpticalModule::OMSimOpticalModule()
{
    mIndex = OMSimHitManager::getInstance().getNextDetectorIndex();
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
    OMSimHitManager::getInstance().setNumberOfPMTs(getNumberOfPMTs(), mIndex);
    mPMTManager->configureSensitiveVolume(pDetConst, getName());
}