/**
 * @file OMSimUIinterface.hh
 * @ingroup common
 */

#ifndef OMSimUIinterface_h
#define OMSimUIinterface_h 1

#include "OMSimCommandArgsTable.hh"
#include <G4UImanager.hh>

/**
 * @class OMSimUIinterface
 * @brief Singleton interface to Geant4's UI manager.
 * @ingroup common
 */
class OMSimUIinterface
{
public:
    // Public static access for the singleton instance
    static OMSimUIinterface &getInstance()
    {
        static OMSimUIinterface instance;
        return instance;
    }

    /**
     * @brief Apply a command to Geant4's UI manager and log it.
     * It accept any number of arguments that are then appended; e.g. applyCommand("cmd", 1, 2, 3) will apply a UI command "cmd 1 2 3"
     * @param pCommand The command string to execute.
     * @param pArgs (Optional) additional arguments that will be appended to the command, one after the other.
     * @tparam Args Variadic template for accepting multiple arguments.
     */
    template <typename... Args>
    void applyCommand(const std::string &pCommand, const Args &...pArgs)
    {
        std::stringstream lStream;
        lStream << pCommand;
        appendToStream(lStream, pArgs...);
        log_trace(lStream.str().c_str());
        mUImanager->ApplyCommand(lStream.str());
    }

    /**
     * @brief Set the UI manager for the OMSimUIinterface class.
     * @param pUI Pointer to the G4UImanager.
     */
    void setUI(G4UImanager *pUI)
    {
        mUImanager = pUI;
    }

    /**
     * @brief Executes a run with a given number of events.
     * @param pNumberOfEvents An optional parameter to specify the number of events to be run.
     * If not provided or less than zero, the number of events is taken from the command arguments table.
     */
    void runBeamOn(G4int pNumberOfEvents = -1)
    {
        log_debug("Running beamOn command");
        G4int lNumEvents = pNumberOfEvents >= 0 ? pNumberOfEvents : OMSimCommandArgsTable::getInstance().get<G4int>("numevents");
        applyCommand("/run/beamOn ", lNumEvents);
    }

private:
    // Private constructor and assignment operator to prevent direct creation or assignment
    OMSimUIinterface() {}
    OMSimUIinterface(const OMSimUIinterface &) = delete;            // disable copying
    OMSimUIinterface &operator=(const OMSimUIinterface &) = delete; // disable assignment

    void appendToStream(std::stringstream &stream)
    {
        // if no additional arguments, do nothing
    }

    /**
     * @brief Append arguments to the command string stream.
     * @param pStream The command string stream.
     * @tparam T Type of the argument to append.
     * @param pVal The value of the argument to append.
     */
    template <typename T>
    void appendToStream(std::stringstream &pStream, const T &pVal)
    {
        pStream << ' ' << pVal;
    }

    /**
     * @brief Append multiple arguments to the command string stream.
     * @param pStream The command string stream.
     * @tparam T Type of the first argument to append.
     * @tparam Args Variadic template for accepting multiple arguments.
     * @param pVal The value of the first argument to append.
     * @param pArgs The values of the additional arguments to append.
     */
    template <typename T, typename... Args>
    void appendToStream(std::stringstream &pStream, const T &pVal, const Args &...pArgs)
    {
        pStream << ' ' << pVal;
        appendToStream(pStream, pArgs...);
    }
    G4UImanager *mUImanager;
};

#endif
//