/**
 * @file OMSimUIinterface.hh
 * @ingroup common
 */

#pragma once

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
    static void init();
    static void shutdown();
    /**
     * @brief Apply a command to Geant4's UI manager and log it.
     * It accept any number of arguments that are then appended; e.g. applyCommand("cmd", 1, 2, 3) will apply a UI command "cmd 1 2 3"
     * @param p_command The command string to execute.
     * @param p_args (Optional) additional arguments that will be appended to the command, one after the other.
     * @tparam Args Variadic template for accepting multiple arguments.
     */
    template <typename... Args>
    void applyCommand(const std::string &p_command, const Args &...p_args)
    {
        std::stringstream stream;
        stream << p_command;
        appendToStream(stream, p_args...);
        log_trace(stream.str().c_str());
        m_uiManager->ApplyCommand(stream.str());
    }

    void setUI(G4UImanager *p_ui);
    void runBeamOn(G4int p_numberOfEvents = -1);

private:
    G4UImanager *m_uiManager;

    OMSimUIinterface() = default;
    ~OMSimUIinterface() = default;
    OMSimUIinterface(const OMSimUIinterface &) = delete;            // disable copying
    OMSimUIinterface &operator=(const OMSimUIinterface &) = delete; // disable assignment

    void appendToStream(std::stringstream &p_stream)
    {
        // if no additional arguments, do nothing
    }

    /**
     * @brief Append arguments to the command string p_stream.
     * @param p_stream The command string p_stream.
     * @tparam T Type of the argument to append.
     * @param p_val The value of the argument to append.
     */
    template <typename T>
    void appendToStream(std::stringstream &p_stream, const T &p_val)
    {
        p_stream << ' ' << p_val;
    }

    /**
     * @brief Append multiple arguments to the command string p_stream.
     * @param p_stream The command string p_stream.
     * @tparam T Type of the first argument to append.
     * @tparam Args Variadic template for accepting multiple arguments.
     * @param p_val The value of the first argument to append.
     * @param p_args The values of the additional arguments to append.
     */
    template <typename T, typename... Args>
    void appendToStream(std::stringstream &p_stream, const T &p_val, const Args &...p_args)
    {
        p_stream << ' ' << p_val;
        appendToStream(p_stream, p_args...);
    }
};

inline OMSimUIinterface *g_OMSimUIinterface = nullptr;