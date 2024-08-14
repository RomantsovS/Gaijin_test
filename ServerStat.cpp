#include "ServerStat.h"

void ServerStat::add(Type type, const std::string &key) {
    std::unique_lock lock(mutex_);
    total_[key][type]++;
    last_stat_[key][type]++;
}

ServerStat::StatType ServerStat::getTotal() const {
    std::unique_lock lock(mutex_);
    return total_;
}

ServerStat::StatType ServerStat::getLastStat() {
    ServerStat::StatType res;
    {
        std::unique_lock lock(mutex_);
        std::swap(res, last_stat_);
    }
    return res;
}
