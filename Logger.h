#pragma once

#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>

class Logger {
public:
    template <typename... Args>
    static void log(Args... args) {
        if (!enabled) {
            return;
        }
        getLogger().log_impl(args...);
    }

    static inline bool enabled = false;

private:
    Logger() : ofs("log.log", std::ios::out) {
        std::cout << __PRETTY_FUNCTION__ << std ::endl;
    }
    static Logger &getLogger() {
        static Logger logger;
        return logger;
    }
    template <typename... Args>
    void log_impl(Args... args) {
        std::unique_lock ul(mtx);
        ofs << "thread id: " << std::this_thread::get_id();
        ((ofs << " " << std::forward<Args>(args)), ...);
        ofs << std::endl;
    }

    std::ofstream ofs;
    std::mutex mtx;
};
