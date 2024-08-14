#pragma once

#include <mutex>
#include <ostream>
#include <string>
#include <unordered_map>

class ServerStat {
public:
    enum class Type { read,
                      write };

    using StatType = std::unordered_map<std::string, std::unordered_map<Type, uint64_t>>;

    void add(Type type, const std::string &key);
    StatType getTotal() const;
    StatType getLastStat();

private:
    mutable std::mutex mutex_;

    StatType total_;
    StatType last_stat_;
};

inline std::ostream &operator<<(std::ostream &os, ServerStat::Type type) {
    switch (type) {
    case ServerStat::Type::read:
        os << "read";
        break;
    case ServerStat::Type::write:
        os << "write";
        break;
    default:
        os << "unknown value";
    };
    return os;
}