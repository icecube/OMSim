/**
 * @file OMSimCommandArgsTable.hh
 * @brief Definition of the OMSimCommandArgsTable singleton class, which controls user args.
 * @ingroup common
 */

#pragma once

/**
 * @brief Writes a key-value pair to a JSON file if the value type matches TYPE.
 * @note This macro is used inside the `OMSimCommandArgsTable::writeToJson()` method.
 */
#define WRITE_TO_JSON_IF_TYPE_MATCHES(VARIANT, TYPE)                   \
    if (type == typeid(TYPE))                                          \
    {                                                                  \
        outputFile << "\t\"" << kv.first << "\": ";                       \
        if (typeid(TYPE) == typeid(std::string))                       \
        {                                                              \
            outputFile << "\"" << boost::any_cast<TYPE>(VARIANT) << "\""; \
        }                                                              \
        else                                                           \
        {                                                              \
            outputFile << boost::any_cast<TYPE>(VARIANT);                 \
        }                                                              \
    }

#include "OMSimLogger.hh"

#include <fstream>
#include <boost/any.hpp>
#include <sys/time.h>
#include <map>

// Forward declaration of the class
class OMSimCommandArgsTable;

// Declaration and definition of the inline global pointer
inline OMSimCommandArgsTable* g_commandArgsTable = nullptr;


/**
 * @class OMSimCommandArgsTable
 * @brief A class used to hold OMSim command arguments with global instance access.
 *
 * This class uses a map to hold key-value pairs of simulation command arguments.
 * It provides a method to write the parameters to a JSON file. Its lifecycle is managed by the OMSim class.
 * 
 * @ingroup common
 */
class OMSimCommandArgsTable
{
    OMSimCommandArgsTable() = default;
    OMSimCommandArgsTable(const OMSimCommandArgsTable &) = delete;
    OMSimCommandArgsTable &operator=(const OMSimCommandArgsTable &) = delete;
    ~OMSimCommandArgsTable() = default;


public:
    using Key = std::string;
    using Value = boost::any; //  Using boost::any to hold any type
private:
    bool m_finalized = false;
    std::map<Key, Value> m_parameters;

public:

    static void init();
    static void shutdown();
    static OMSimCommandArgsTable &getInstance();
    void setParameter(const Key &p_key, const Value &p_value);
    bool keyExists(const Key &p_key); 
    void writeToJson(std::string p_fileName);
    void finalize();
    
    /**
     * @brief Retrieves a parameter from the table.
     * @param p_key The key for the parameter.
     * @return The parameter value.
     * @throw std::invalid_argument If the parameter is not of type T, or if the key does not exist.
     */
    template <typename T>
    T get(const std::string &p_key)
    {
        try
        {
            return boost::any_cast<T>(m_parameters.at(p_key));
        }
        catch (const boost::bad_any_cast &e)
        {
            log_error(("Failed to get parameter '" + p_key + "' as type " + typeid(T).name()).c_str());
            throw std::invalid_argument("Failed to get parameter " + p_key + " as type " + typeid(T).name());
        }
        catch (const std::out_of_range &e)
        {
            log_error(("Parameter '" + p_key + "'does not exist").c_str());
            throw std::invalid_argument("Parameter '" + p_key + "' does not exist");
        }
    }

};
