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
    template <typename T>
    void appendToStream(std::stringstream &stream, const T &val)
    {
        stream << ' ' << val;
    }

    void appendToStream(std::stringstream &stream)
    {
        // do nothing
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