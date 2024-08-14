#pragma once

#include <array>
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "Logger.h"

namespace ba = boost::asio;

class Client {
public:
    Client(ba::io_context &io_context, short port, std::vector<std::string> queries)
        : socket_{io_context},
          queries_(std::move(queries)) {
        Logger::log("cli: Client ctor");
        ba::ip::tcp::endpoint ep(ba::ip::address::from_string("127.0.0.1"), port);

        socket_.async_connect(ep, [&](const boost::system::error_code &erc) {
            connectHandler(erc);
        });
    }
    ~Client() {
        Logger::log("cli: Client dtor");
    }

private:
    void connectHandler(const boost::system::error_code &ec) {
        Logger::log("cli: connectHandler");
        if (ec) {
            Logger::log("cli: connectHandler - failed! err = ", ec.message());
            return;
        }

        for (auto &query : queries_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
            Logger::log("cli: send ", query);
            ba::write(socket_, ba::buffer(query));
            ba::read_until(socket_, read_buffer_, '\n');
            std::string answer;
            std::istream is(&read_buffer_);
            std::getline(is, answer);
            Logger::log("cli: read: ", answer);
            std::cout << "cli: answer: " << answer << '\n';
        }
        boost::system::error_code erc;
        socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, erc);
        socket_.close();
    }

    ba::ip::tcp::socket socket_;
    std::vector<std::string> queries_;
    ba::streambuf read_buffer_;
};