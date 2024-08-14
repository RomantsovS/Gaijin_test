#include "Core.h"
#include "Logger.h"
#include <sstream>

Core::Core(Database &db) : db_(db) {
    thread_ = std::thread(&Core::printStat, this);
}

Core::~Core() {
    Stop();
}

void Core::setDBWriter(std::unique_ptr<DatabaseWriter> db_writer) {
    db_writer_ = std::move(db_writer);
}

std::string Core::handleMessage(const std::string &msg) {
    std::istringstream iss(msg);
    std::string cmd, key;

    iss >> cmd;
    if (cmd.size() != 4 || cmd[0] != '$') {
        return "FAIL incorrect command";
    }

    if (cmd.substr(1) == "get") {
        iss >> key;
        server_stat_.add(ServerStat::Type::read, key);
        if (!iss.eof()) {
            return "FAIL incorrect command";
        }
        std::ostringstream oss;
        oss << key << '=';
        auto pair = db_.get(key);
        if (pair.second) {
            oss << pair.first;
        }
        return oss.str();
    }
    if (cmd.substr(1) == "set") {
        std::string line;
        iss >> line;
        auto pos = line.find('=');
        if (pos == std::string::npos) {
            return "FAIL incorrect command";
        }
        key = line.substr(0, pos);
        server_stat_.add(ServerStat::Type::write, key);

        auto value = line.substr(pos + 1);
        auto k = key;
        db_.set(std::move(key), std::move(value));
        if (db_writer_) {
            db_writer_->AddTask(std::move(k));
        }
        return line;
    }
    return "FAIL incorrect command";
}

void Core::Stop() {
    if (stopped_) {
        return;
    }
    if (db_writer_) {
        db_writer_->Stop();
    }
    stopped_ = true;
    cv_.notify_one();
    thread_.join();
}

void Core::printStat() {
    Logger::log("srv: Core printStat started");

    while (!stopped_) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, std::chrono::seconds(5), [this] { return stopped_; });

        std::cout << "total stat:\n";

        auto total = server_stat_.getTotal();

        for (const auto &[key, stat] : total) {
            std::cout << key << ":\n";
            for (const auto &[type, num] : stat) {
                std::cout << type << '=' << num << '\n';
            }
        }

        std::cout << "last 5 sec stat:\n";

        auto last_stat = server_stat_.getLastStat();

        for (const auto &[key, stat] : last_stat) {
            std::cout << key << ":\n";
            for (const auto &[type, num] : stat) {
                std::cout << type << '=' << num << '\n';
            }
        }

        if (stopped_) {
            break;
        }
    }

    Logger::log("srv: Core printStat finished");
}
