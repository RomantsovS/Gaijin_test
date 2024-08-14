#include "Database.h"
#include <chrono>
#include <fstream>
#include <sstream>

#include "Logger.h"

void Database::set(std::string &&key, std::string &&value) {
    std::unique_lock lock(mutex_);
    data[std::move(key)] = std::move(value);
}

std::pair<std::string, bool> Database::get(const std::string &key) const {
    std::shared_lock lock(mutex_);
    auto iter = data.find(key);
    if (iter == data.end()) {
        return {"", false};
    }
    return {iter->second, true};
}

bool DatabaseReader::ReadFromFile(Database &db, const std::filesystem::path &filename) {
    if (!std::filesystem::exists(filename)) {
        return false;
    }
    auto file = std::ifstream(filename);
    if (!file) {
        return false;
    }
    return ReadData(db, file);
}

bool DatabaseReader::ReadData(Database &db, std::istream &is) {
    std::string line;
    while (std::getline(is, line)) {
        std::string key;
        std::string value;
        std::istringstream ss(line);

        ss >> key >> value;
        db.set(std::move(key), std::move(value));
    }
    return true;
}

DatabaseWriter::DatabaseWriter(Database &db, std::filesystem::path filename) : db_(db), filename_(filename) {
    thread_ = std::thread(&DatabaseWriter::do_work, this);
}

DatabaseWriter::~DatabaseWriter() {
    Stop();
}

void DatabaseWriter::Stop() {
    if (stopped_) {
        return;
    }
    Logger::log("srv: DatabaseWriter stop");
    stopped_ = true;
    cv_.notify_all();
    thread_.join();
    Logger::log("srv: DatabaseWriter stopped");
}

void DatabaseWriter::AddTask(std::string &&key) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        updated_keys_.insert(std::move(key));
    }
    // cv_.notify_one();
}

void DatabaseWriter::do_work() {
    Logger::log("srv: DatabaseWriter started");

    while (true) {
        std::unordered_set<std::string> tmp;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait_for(lock, std::chrono::seconds(2), [this] { return stopped_ || !updated_keys_.empty(); });

            if (stopped_) {
                break;
            }

            if (updated_keys_.empty()) {
                continue;
            }

            std::swap(tmp, updated_keys_);
        }

        saveConfig(std::move(tmp));
    }

    Logger::log("srv: DatabaseWriter finished");
}

void DatabaseWriter::saveConfig(std::unordered_set<std::string> &&updated_keys) {
    auto tmp_filename = filename_;
    tmp_filename += ".tmp";
    std::ofstream ofs(tmp_filename, std::ios::out);
    if (!ofs.is_open()) {
        throw std::runtime_error("can't open tmp file");
    }
    std::ifstream ifs(filename_);

    std::string line;
    while (std::getline(ifs, line)) {
        std::string key;
        std::istringstream ss(line);

        ss >> key;
        if (updated_keys.count(key) == 0) {
            Logger::log("srv: DatabaseWriter save old line: ", line);
            ofs << line;
        } else {
            Logger::log("srv: DatabaseWriter save updated: ", key);
            ofs << key << ' ' << db_.get(key).first;
        }
        ofs << '\n';
        updated_keys.erase(key);
    }
    for (const auto &key : updated_keys) {
        Logger::log("srv: DatabaseWriter save new key: ", key);
        ofs << key << ' ' << db_.get(key).first << '\n';
    }
    ifs.close();
    ofs.close();

    std::filesystem::remove(filename_);
    std::filesystem::rename(tmp_filename, filename_);
}
