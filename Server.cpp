#include "Server.h"
#include "Core.h"
#include "Logger.h"

session::session(ba::ip::tcp::socket socket, Core &core)
    : socket_(std::move(socket)), core_(core) {
    Logger::log("srv: session started");
}

session::~session() {
    Logger::log("srv: session finished");
}

void session::do_read() {
    Logger::log("srv: session do_read");
    auto self(shared_from_this());
    ba::async_read_until(socket_,
                         read_buffer_,
                         '\n',
                         [this, self](boost::system::error_code ec, size_t length) {
                             if (!ec) {
                                 std::string msg;
                                 std::istream is(&read_buffer_);
                                 std::getline(is, msg);
                                 Logger::log("srv: receive ", length, "=", msg);
                                 auto reply = core_.handleMessage(msg);
                                 do_write(reply);
                             } else if (ec != ba::error::eof) {
                                 Logger::log("srv: session receive error:", ec.message());
                             }
                         });
}

void session::do_write(const std::string &msg) {
    Logger::log("srv: session do_write");
    auto end = msg.end();
    if (msg.size() >= max_length - 1) {
        Logger::log("srv: session do_write msg is loo big");
        end = std::next(msg.begin(), max_length - 1);
    }
    std::copy(msg.begin(), end, write_buffer_);
    write_buffer_[msg.size()] = '\n';
    auto self(shared_from_this());
    Logger::log("srv: send ", msg);
    ba::async_write(socket_,
                    ba::buffer(write_buffer_, msg.size() + 1),
                    [this, self](boost::system::error_code ec, [[maybe_unused]] size_t length) {
                        if (ec) {
                            Logger::log("srv: session write error:", ec.message());
                        }
                        Logger::log("srv: sent ", length, " bytes");
                        do_read();
                    });
}

Server::Server(short port, Core &core, size_t num_threads)
    : num_threads_(num_threads), acceptor_(io_context_, ba::ip::tcp::endpoint(ba::ip::tcp::v4(), port)), core_(core) {
    Logger::log("srv: Server ctor");
}

Server::~Server() {
    Logger::log("srv: Server dtor");
    Stop();
}

void Server::Run() {
    Logger::log("srv: Server Run");
    do_accept();

    for (size_t i = 0; i < num_threads_; ++i) {
        pool.emplace_back([this]() { io_context_.run(); });
    }
    for (auto &thread : pool) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    Logger::log("srv: Server Run end");
}

void Server::Stop() {
    if (stopped_) {
        return;
    }
    Logger::log("srv: Server stop");
    acceptor_.close();
    stopped_ = true;
    core_.Stop();
    Logger::log("srv: Server stopped");
}

void Server::do_accept() {
    Logger::log("srv: server do_accept");
    acceptor_.async_accept([this](boost::system::error_code ec, ba::ip::tcp::socket socket) {
        if (ec) {
            Logger::log("srv: accept - failed! err = ", ec.message());
        } else {
            if (!stopped_) {
                Logger::log("srv: accepted ", socket.remote_endpoint());

                std::make_shared<session>(std::move(socket), core_)->start();
                do_accept();
            } else {
                Logger::log("srv: Too late - the server was stopped\n");
            }
        }
    });
}