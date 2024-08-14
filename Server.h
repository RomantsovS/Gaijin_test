#pragma once

#include <boost/asio.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "Core.h"
#include "Database.h"

namespace ba = boost::asio;

class session : public std::enable_shared_from_this<session> {
public:
    session(ba::ip::tcp::socket socket, Core &core);
    ~session();

    void start() { do_read(); }

private:
    void do_read();
    void do_write(const std::string &msg);

    ba::ip::tcp::socket socket_;
    ba::streambuf read_buffer_;
    enum { max_length = 1024 };
    char write_buffer_[max_length];
    Core &core_;
};

class Server {
public:
    Server(short port, Core &core, size_t num_threads = 1);
    ~Server();

    void Run();
    void Stop();

private:
    void do_accept();

    boost::asio::io_context io_context_;
    size_t num_threads_;
    ba::ip::tcp::acceptor acceptor_;
    bool stopped_ = false;
    Core &core_;
    std::vector<std::thread> pool;
};
