/**
 * @file OMSimCommandArgsTable.hh
 * @brief Definition of the OMSimCommandArgsTable singleton class, which controls user args.
 * @ingroup common
 */

#ifndef OMSIMCOMMANDARGSTABLE_H
#define OMSIMCOMMANDARGSTABLE_H

/**
 * @brief Writes a key-value pair to a JSON file if the value type matches TYPE.
 * @note This macro is used inside the `OMSimCommandArgsTable::writeToJson()` method.
 */
#define WRITE_TO_JSON_IF_TYPE_MATCHES(VARIANT, TYPE)                   \
    if (type == typeid(TYPE))                                          \
    {                                                                  \
        lOutputFile << "\t\"" << kv.first << "\": ";                       \
        if (typeid(TYPE) == typeid(std::string))                       \
        {                                                              \
            lOutputFile << "\"" << boost::any_cast<TYPE>(VARIANT) << "\""; \
        }                                                              \
        else                                                           \
        {                                                              \
            lOutputFile << boost::any_cast<TYPE>(VARIANT);                 \
        }                                                              \
    }

#include "OMSimLogger.hh"

#include <fstream>
#include <boost/any.hpp>
#include <sys/time.h>

/**
 * @class OMSimCommandArgsTable
 * @brief A singleton class used to hold OMSim command arguments.
 *
 * This class uses a map to hold key-value pairs of simulation command arguments. In principle it is just a wrapper around the map created by the boost library to avoid users changing the arg values after initialisation.
 * Provides a method to write the parameters to a JSON file.
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

    /**
     * @brief Retrieves the instance of the singleton.
     * @return The instance of OMSimCommandArgsTable.
     */
    static OMSimCommandArgsTable &getInstance()
    {
        static OMSimCommandArgsTable instance;
        return instance;
    }

    /**
     * @brief Sets a parameter in the arg table.
     * @param pKey The key for the parameter.
     * @param pValue The value for the parameter.
     * @throw std::runtime_error If the table is already finalized (i.e. somebody is trying to set a new arg parameter after args were parsed (?!)).
     */
    void setParameter(const Key &pKey, const Value &pValue)
    {
        if (mFinalized)
        {
            throw std::runtime_error("Cannot modify OMSimCommandArgsTable after it's been finalized!");
        }
        mParameters[pKey] = pValue;
    }

    /**
     * @brief Retrieves a parameter from the table.
     * @param pKey The key for the parameter.
     * @return The parameter value.
     * @throw std::invalid_argument If the parameter is not of type T, or if the key does not exist.
     */
    template <typename T>
    T get(const std::string &pKey)
    {
        try
        {
            return boost::any_cast<T>(mParameters.at(pKey));
        }
        catch (const boost::bad_any_cast &e)
        {
            log_error(("Failed to get parameter " + pKey + " as type " + typeid(T).name()).c_str());
            throw std::invalid_argument("Failed to get parameter " + pKey + " as type " + typeid(T).name());
        }
        catch (const std::out_of_range &e)
        {
            log_error(("Parameter " + pKey + " does not exist").c_str());
            throw std::invalid_argument("Parameter " + pKey + " does not exist");
        }
    }

    bool keyExists(const Key &pKey) const
    {
        return mParameters.find(pKey) != mParameters.end();
    }

    /**
     * @brief Writes the parameters to a JSON-formatted file.
     * @param pFileName The name of the JSON file.
     * @throw std::runtime_error If the file fails to open.
     */
    void writeToJson(std::string pFileName)
    {
        std::ofstream lOutputFile(pFileName);

        if (!lOutputFile.is_open())
        {
            throw std::runtime_error("Failed to open file " + pFileName);
        }

        lOutputFile << "{\n";
        size_t lCount = mParameters.size();
        for (const auto &kv : mParameters)
        {
            --lCount; // Decrease count for each iteration

            if (kv.second.empty())
            {
                lOutputFile << "\t\"" << kv.first << "\": \"\"";
            }
            else
            {
                const std::type_info &type = kv.second.type();

                WRITE_TO_JSON_IF_TYPE_MATCHES(kv.second, int)
                WRITE_TO_JSON_IF_TYPE_MATCHES(kv.second, double)
                WRITE_TO_JSON_IF_TYPE_MATCHES(kv.second, long)
                WRITE_TO_JSON_IF_TYPE_MATCHES(kv.second, bool)
                WRITE_TO_JSON_IF_TYPE_MATCHES(kv.second, std::string)
            }

            // Append comma if it's not the last item
            if (lCount != 0)
            {
                lOutputFile << ",";
            }

            lOutputFile << "\n";
        }
        lOutputFile << "}\n";
        lOutputFile.close();
    }

    /**
     * @brief Finalizes the table, setting a random seed if none was provided. mFinalized is set to true preventing any further modifications.
     */
    void finalize()
    {
        long lSeed;
        if (!keyExists("seed"))
        {
            struct timeval time_for_randy;
            gettimeofday(&time_for_randy, NULL);
            lSeed = time_for_randy.tv_sec + 4294 * time_for_randy.tv_usec;
            setParameter("seed", lSeed);
        }
        mFinalized = true;
    }

private:
    bool mFinalized = false;
    std::map<Key, Value> mParameters;
};

#endif // OMSIMCOMMANDARGSTABLE_H
