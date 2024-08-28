#include "OMSimUIinterface.hh"

/**
 * @brief Initializes the global instance of OMSimUIinterface
 * This method is normally called in OMSim::initialiseSimulation.
 */
void OMSimUIinterface::init()
{
    if (!g_OMSimUIinterface)
    {
        g_OMSimUIinterface = new OMSimUIinterface();
    }
}

/**
 * @brief Deletes the global instance of OMSimUIinterface
 * This method is normally called in the destructor ~OMSim.
 */
void OMSimUIinterface::shutdown()
{
    delete g_OMSimUIinterface;
    g_OMSimUIinterface = nullptr;
}

/**
 * @brief Set the UI manager for the OMSimUIinterface class.
 * @param p_ui Pointer to the G4UImanager.
 */
void OMSimUIinterface::setUI(G4UImanager *p_ui)
{
    m_uiManager = p_ui;
}

/**
 * @brief Executes a run with a given number of events.
 * @param p_numberOfEvents An optional parameter to specify the number of events to be run.
 * If not provided or less than zero, the number of events is taken from the command arguments table.
 */
void OMSimUIinterface::runBeamOn(G4int p_numberOfEvents)
{
    log_trace("Running beamOn command");
    G4int numberOfEvents = p_numberOfEvents >= 0 ? p_numberOfEvents : OMSimCommandArgsTable::getInstance().get<G4int>("numevents");
    applyCommand("/run/beamOn ", numberOfEvents);
}