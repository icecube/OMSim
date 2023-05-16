#ifndef OMSIMCOMMANDARGSTABLE_H
#define OMSIMCOMMANDARGSTABLE_H

#include <map>
#include <variant>
#include <string>
#include <stdexcept>
#include "globals.hh"
#include <boost/any.hpp>

class OMSimCommandArgsTable {
public:
    using Key = std::string;
    using Value = boost::any;  //  Using boost::any to hold any type

    static OMSimCommandArgsTable& getInstance() {
        static OMSimCommandArgsTable instance;
        return instance;
    }

    void setParameter(const Key& key, const Value& value) {
        if (mFinalized) {
            throw std::runtime_error("Cannot modify OMSimCommandArgsTable after it's been finalized!");
        }
        mParameters[key] = value;
    }

    template <typename T>
    T get(const std::string& key) {
        try {
            return boost::any_cast<T>(mParameters.at(key));
        } catch (const boost::bad_any_cast& e) {
            throw std::invalid_argument("Failed to get parameter " + key + " as type " + typeid(T).name());
        } catch (const std::out_of_range& e) {
            throw std::invalid_argument("Parameter " + key + " does not exist");
        }
    }

    void finalize() {
        mFinalized = true;
    }

private:
    OMSimCommandArgsTable() = default;
    ~OMSimCommandArgsTable() = default;
    OMSimCommandArgsTable(const OMSimCommandArgsTable&) = delete;
    OMSimCommandArgsTable& operator=(const OMSimCommandArgsTable&) = delete;

    bool mFinalized = false;
    std::map<Key, Value> mParameters;
};

#endif // OMSIMCOMMANDARGSTABLE_H
