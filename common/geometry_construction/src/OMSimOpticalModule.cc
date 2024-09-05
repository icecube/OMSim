#include "OMSimOpticalModule.hh"
#include "OMSimHitManager.hh"
#include "OMSimPMTConstruction.hh"

OMSimOpticalModule::OMSimOpticalModule(OMSimPMTConstruction *p_PMTManager) : OMSimDetectorComponent(), m_managerPMT(p_PMTManager)
{
    m_index = OMSimHitManager::getInstance().getNextDetectorIndex();
    log_trace("Constructor of OMSimOpticalModule");
}

OMSimOpticalModule::~OMSimOpticalModule()
{
    if (m_managerPMT != nullptr)
    {
        delete m_managerPMT;
        m_managerPMT = nullptr;
    }
}

OMSimPMTConstruction *OMSimOpticalModule::getPMTmanager()
{
    log_trace("Getting PMT instance used in optical module instance");
    return m_managerPMT;
}

void OMSimOpticalModule::configureSensitiveVolume(OMSimDetectorConstruction *pDetConst)
{   
    log_debug("Configuring {} as sensitive detector", getName());
    OMSimHitManager::getInstance().setNumberOfPMTs(getNumberOfPMTs(), m_index);
    m_managerPMT->configureSensitiveVolume(pDetConst, getName());
}