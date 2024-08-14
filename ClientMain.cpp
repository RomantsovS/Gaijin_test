#include <future>
#include <iostream>

#include "Client.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
    std::vector<std::string> keys;
    for (int i = 0; i < 10; ++i) {
        std::ostringstream oss;
        oss << "key" << i;
        keys.emplace_back(oss.str());
    }

    std::vector<std::string> queries;
    for (int i = 0; i < 10000; ++i) {
        std::ostringstream oss;
        if (rand() % 10 == 0) {
            oss << "$set " << keys[rand() % keys.size()] << "=value" << rand() % 1000 << '\n';
        } else {
            oss << "$get " << keys[rand() % keys.size()] << '\n';
        }

        queries.emplace_back(oss.str());
    }

    ba::io_context io_context_client;
    Client client(io_context_client, 7899, std::move(queries));
    io_context_client.run();

    return 0;
}