#include "OMSimCommandArgsTable.hh"

/**
 * @return A reference to the OMSimCommandArgsTable instance.
 * @throw std::runtime_error if accessed before initialization or after shutdown.
 */
OMSimCommandArgsTable &OMSimCommandArgsTable::getInstance()
{
    if (!g_commandArgsTable)
        throw std::runtime_error("OMSimCommandArgsTable accessed before initialization or after shutdown!");
    return *g_commandArgsTable;
}

/**
 * @brief Initializes the global instance of OMSimCommandArgsTable.
 *
 * This method is normally called in the constructor of OMSim.
 */
void OMSimCommandArgsTable::init()
{
    if (!g_commandArgsTable)
        g_commandArgsTable = new OMSimCommandArgsTable();
}

/**
 * @brief Deletes the global instance of OMSimCommandArgsTable.
 *
 * This method is normally called in the destructor ~OMSim.
 */
void OMSimCommandArgsTable::shutdown()
{
    delete g_commandArgsTable;
    g_commandArgsTable = nullptr;
}

/**
 * @brief Sets a parameter in the arg table.
 * @param p_key The key for the parameter.
 * @param p_value The value for the parameter.
 * @throw std::runtime_error If the table is already finalized (i.e. somebody is trying to set a new arg parameter after args were parsed (?!)).
 */
void OMSimCommandArgsTable::setParameter(const Key &p_key, const Value &p_value)
{
    if (m_finalized)
    {
        throw std::runtime_error("Cannot modify OMSimCommandArgsTable after it's been finalized!");
    }
    m_parameters[p_key] = p_value;
}


bool OMSimCommandArgsTable::keyExists(const Key &p_key)
{
    return m_parameters.find(p_key) != m_parameters.end();
}

/**
 * @brief Writes the parameters to a JSON-formatted file.
 * @param p_fileName The name of the JSON file.
 * @throw std::runtime_error If the file fails to open.
 */
void OMSimCommandArgsTable::writeToJson(std::string p_fileName)
{
    std::ofstream outputFile(p_fileName);

    if (!outputFile.is_open())
    {
        throw std::runtime_error("Failed to open file " + p_fileName);
    }

    outputFile << "{\n";
    size_t count = m_parameters.size();
    for (const auto &kv : m_parameters)
    {
        --count; // Decrease count for each iteration

        if (kv.second.empty())
        {
            outputFile << "\t\"" << kv.first << "\": \"\"";
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
        if (count != 0)
        {
            outputFile << ",";
        }

        outputFile << "\n";
    }
    outputFile << "}\n";
    outputFile.close();
}

/**
 * @brief Finalizes the table, setting a random seed if none was provided. m_finalized is set to true preventing any further modifications.
 */
void OMSimCommandArgsTable::finalize()
{
    long seed;
    if (!keyExists("seed"))
    {
        struct timeval timeForRandom;
        gettimeofday(&timeForRandom, NULL);
        seed = timeForRandom.tv_sec + 4294 * timeForRandom.tv_usec;
        setParameter("seed", seed);
    }
    m_finalized = true;
}