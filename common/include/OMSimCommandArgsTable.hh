#ifndef OMSIMCOMMANDARGSTABLE_H
#define OMSIMCOMMANDARGSTABLE_H
#define WRITE_TO_JSON_IF_TYPE_MATCHES(VARIANT, TYPE)  \
    if (type == typeid(TYPE)) {                       \
        outFile << "\t\"" << kv.first << "\": "       \
        << boost::any_cast<TYPE>(VARIANT) << ",\n";   \
        continue;                                     \
    }

#include <map>
#include <variant>
#include <string>
#include <stdexcept>
#include "globals.hh"
#include <boost/any.hpp>

class OMSimCommandArgsTable
{
public:
    using Key = std::string;
    using Value = boost::any; //  Using boost::any to hold any type

    static OMSimCommandArgsTable &getInstance()
    {
        static OMSimCommandArgsTable instance;
        return instance;
    }

    void setParameter(const Key &key, const Value &value)
    {
        if (mFinalized)
        {
            throw std::runtime_error("Cannot modify OMSimCommandArgsTable after it's been finalized!");
        }
        mParameters[key] = value;
    }

    template <typename T>
    T get(const std::string &key)
    {
        try
        {
            return boost::any_cast<T>(mParameters.at(key));
        }
        catch (const boost::bad_any_cast &e)
        {
            throw std::invalid_argument("Failed to get parameter " + key + " as type " + typeid(T).name());
        }
        catch (const std::out_of_range &e)
        {
            throw std::invalid_argument("Parameter " + key + " does not exist");
        }
    }

    bool keyExists(const Key &key) const
    {
        return mParameters.find(key) != mParameters.end();
    }

    void writeToJson(std::string lFileName)
    {
        std::ofstream outFile(lFileName);

        if (!outFile.is_open())
        {
            throw std::runtime_error("Failed to open file " + lFileName);
        }

        outFile << "{\n";
        for (const auto &kv : mParameters)
        {
            if (kv.second.empty())
            {
                outFile << "\t\"" << kv.first << "\": \"\",\n";
                continue;
            }

            const std::type_info &type = kv.second.type();

            WRITE_TO_JSON_IF_TYPE_MATCHES(kv.second, int)
            WRITE_TO_JSON_IF_TYPE_MATCHES(kv.second, double)
            WRITE_TO_JSON_IF_TYPE_MATCHES(kv.second, long)
            WRITE_TO_JSON_IF_TYPE_MATCHES(kv.second, bool)
            WRITE_TO_JSON_IF_TYPE_MATCHES(kv.second, std::string)
        }
        outFile << "}\n";
        outFile.close();
    }

    void finalize()
    {
        mFinalized = true;
    }

private:
    OMSimCommandArgsTable() = default;
    ~OMSimCommandArgsTable() = default;
    OMSimCommandArgsTable(const OMSimCommandArgsTable &) = delete;
    OMSimCommandArgsTable &operator=(const OMSimCommandArgsTable &) = delete;

    bool mFinalized = false;
    std::map<Key, Value> mParameters;
};

#endif // OMSIMCOMMANDARGSTABLE_H
