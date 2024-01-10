/**
 * @file OMSimUIinterface.hh
 * @brief The OMSimUIinterface class provides a singleton interface to Geant4's UI manager.
 * @details This class provides a singleton interface to Geant4's UI manager. It simplifies the application of commands to the UI manager by handling argument parsing
 *          and logging. The class is a singleton, meaning there can only be one instance of it. The instance can be accessed using the `getInstance()` static member function.
 * @author Martin Unland
 * @ingroup common
 */

#ifndef OMSimUIinterface_h
#define OMSimUIinterface_h 1

#include "OMSimCommandArgsTable.hh"
#include <G4UImanager.hh>


/**
 * @class OMSimUIinterface
 * @brief Singleton interface to Geant4's UI manager.
 * @details The OMSimUIinterface class provides a singleton interface to Geant4's UI manager.
 *          It simplifies the application of commands to the UI manager by handling argument parsing
 *          and logging.
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
     * @details It accept any number of arguments that are then appended; e.g. applyCommand("cmd", 1, 2, 3) will apply a UI command "cmd 1 2 3"
     * @param command The command string to execute.
     * @param args (Optional) additional arguments that will be appended to the command, one after the other.
     * @tparam Args Variadic template for accepting multiple arguments.
     */
    template <typename... Args>
    void applyCommand(const std::string &command, const Args &...args)
    {
        std::stringstream stream;
        stream << command;
        appendToStream(stream, args...);
        log_trace(stream.str().c_str());
        UI->ApplyCommand(stream.str());
    }

    /**
     * @brief Set the UI manager for the OMSimUIinterface class. This is done in the main.
     * @param pUI Pointer to the G4UImanager.
     */
    void setUI(G4UImanager *pUI)
    {
        UI = pUI;
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
     * @brief Helper function to append arguments to the command string stream.
     * @param stream The command string stream.
     * @tparam T Type of the argument to append.
     * @param val The value of the argument to append.
     */
    template <typename T>
    void appendToStream(std::stringstream &stream, const T &val)
    {
        stream << ' ' << val;
    }

    /**
     * @brief Variadic template function to append multiple arguments to the command string stream.
     * @param stream The command string stream.
     * @tparam T Type of the first argument to append.
     * @tparam Args Variadic template for accepting multiple arguments.
     * @param val The value of the first argument to append.
     * @param args The values of the additional arguments to append.
     */
    template <typename T, typename... Args>
    void appendToStream(std::stringstream &stream, const T &val, const Args &...args)
    {
        stream << ' ' << val;
        appendToStream(stream, args...);
    }
    G4UImanager *UI;
};

#endif
//