#pragma once

#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

class Database {
public:
    void set(std::string &&key, std::string &&value);
    std::pair<std::string, bool> get(const std::string &key) const;

private:
    std::unordered_map<std::string, std::string> data;
    mutable std::shared_mutex mutex_;
};

class DatabaseReader {
public:
    static bool ReadFromFile(Database &db, const std::filesystem::path &filename);
    static bool ReadData(Database &db, std::istream &is);
};

class DatabaseWriter {
public:
    DatabaseWriter(Database &db, std::filesystem::path filename);
    ~DatabaseWriter();

    void Stop();

    void AddTask(std::string &&key);

private:
    void do_work();
    void saveConfig(std::unordered_set<std::string> &&updated_keys);

    Database &db_;
    std::filesystem::path filename_;

    std::thread thread_;
    bool stopped_ = false;

    std::mutex mutex_;
    std::condition_variable cv_;

    std::unordered_set<std::string> updated_keys_;
};