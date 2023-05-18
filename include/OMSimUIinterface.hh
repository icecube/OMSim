/**
 * @file OMSimUIinterface.hh
 * @details The OMSimUIinterface class provides a singleton interface to Geant4's UI manager.
 *          It simplifies the application of commands to the UI manager by handling argument parsing 
 *          and logging.
 */

#ifndef OMSimUIinterface_h
#define OMSimUIinterface_h 1

#include <sstream>
#include <string>
#include <G4UImanager.hh>
#include "OMSimLogger.hh"

class OMSimUIinterface
{
public:
    // Public static access for the singleton instance
    static OMSimUIinterface& getInstance()
    {
        static OMSimUIinterface instance;
        return instance;
    }

    /**
     * @brief Apply a command to Geant4's UI manager and log it.
     * @param command The command string to execute.
     * @param args (Optional) additional arguments that will be appended to the command, one after the other.
     * e.g. applyCommand("cmd", 1, 2, 3) will apply a UI command "cmd 1 2 3"
     */
    template <typename... Args>
    void applyCommand(const std::string &command, const Args &...args)
    {
        std::stringstream stream;
        stream << command;
        appendToStream(stream, args...);
        UI->ApplyCommand(stream.str());
        log_debug(stream.str().c_str());
    }
    void setUI(G4UImanager* pUI){
        UI = pUI;
        }

private:
    // Private constructor and assignment operator to prevent direct creation or assignment
    OMSimUIinterface() {} 
    OMSimUIinterface(const OMSimUIinterface&) = delete; // disable copying
    OMSimUIinterface& operator=(const OMSimUIinterface&) = delete; // disable assignment

    void appendToStream(std::stringstream &stream)
    {
        // if no additional arguments, do nothing
    }

    template <typename T>
    void appendToStream(std::stringstream &stream, const T &val)
    {
        stream << ' ' << val;
    }

    template <typename T, typename... Args>
    void appendToStream(std::stringstream &stream, const T &val, const Args &...args)
    {
        stream << ' ' << val;
        appendToStream(stream, args...);
    }
    G4UImanager* UI;
};


#endif
//