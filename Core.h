#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "Database.h"
#include "ServerStat.h"

class Core {
public:
    Core(Database &db);
    ~Core();

    void setDBWriter(std::unique_ptr<DatabaseWriter> db_writer);

    std::string handleMessage(const std::string &msg);

    void Stop();

private:
    void printStat();

    Database &db_;
    std::unique_ptr<DatabaseWriter> db_writer_;
    ServerStat server_stat_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool stopped_ = false;
    std::thread thread_;
};