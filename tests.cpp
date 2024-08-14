#include <gtest/gtest.h>

#include <chrono>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "Client.h"
#include "Core.h"
#include "Database.h"
#include "Logger.h"
#include "Server.h"

TEST(TestAll, TestDB) {
    Database db;

    std::istringstream is("key1 value1\nkey2 value2\n");
    DatabaseReader::ReadData(db, is);

    auto p = db.get("key1");
    EXPECT_EQ(p.first, "value1");
    EXPECT_TRUE(p.second);
    p = db.get("key2");
    EXPECT_EQ(p.first, "value2");
    EXPECT_TRUE(p.second);
    p = db.get("key3");
    EXPECT_EQ(p.first, "");
    EXPECT_FALSE(p.second);

    db.set("key2", "value3");
    p = db.get("key2");
    EXPECT_EQ(p.first, "value3");
}

TEST(TestAll, TestCore) {
    Database db;
    Core core(db);

    EXPECT_EQ(core.handleMessage(""), "FAIL incorrect command");
    EXPECT_EQ(core.handleMessage("get key"), "FAIL incorrect command");
    EXPECT_EQ(core.handleMessage("$get key value"), "FAIL incorrect command");

    EXPECT_EQ(core.handleMessage("$set key1=value1"), "key1=value1");
    EXPECT_EQ(core.handleMessage("$get key1"), "key1=value1");

    EXPECT_EQ(core.handleMessage("$get key2"), "key2=");
}

TEST(TestAll, Test1) {
    Logger::enabled = true;
    Database db;
    Core core(db);

    const std::filesystem::path filename = "config_test.txt";
    std::ofstream ofs(filename, std::ios::out);
    ASSERT_TRUE(ofs.is_open());
    ofs << "key0 value0\n";
    ofs.close();

    std::ifstream ifs(filename);
    ASSERT_TRUE(ifs.is_open());
    DatabaseReader::ReadData(db, ifs);
    ifs.close();

    auto p = db.get("key0");
    EXPECT_EQ(p.first, "value0");
    EXPECT_TRUE(p.second);

    core.setDBWriter(std::move(std::make_unique<DatabaseWriter>(db, filename)));

    Server server(7899, core, 4);
    std::thread server_thread([&server]() { server.Run(); });

    auto client_lambda = []() {
        ba::io_context io_context_client;

        std::vector<std::string> queries;
        for (int i = 0; i < 10; ++i) {
            std::ostringstream oss;
            oss << "$set key" << i << "=value" << i << '\n';
            queries.emplace_back(oss.str());
        }
        for (int i = 0; i < 10; ++i) {
            std::ostringstream oss;
            oss << "$get key" << i << '\n';
            queries.emplace_back(oss.str());
        }
        Client client(io_context_client, 7899, std::move(queries));
        io_context_client.run();
    };
    std::vector<std::thread> client_threads;
    for (int i = 0; i < 4; ++i) {
        client_threads.emplace_back(client_lambda);
    }
    for (auto &thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    std::this_thread::sleep_for(std::chrono::seconds(15));

    server.Stop();
    server_thread.join();

    p = db.get("key0");
    EXPECT_TRUE(p.second);
    EXPECT_EQ(p.first, "value0");
    p = db.get("key9");
    EXPECT_TRUE(p.second);
    EXPECT_EQ(p.first, "value9");

    p = db.get("key10");
    EXPECT_FALSE(p.second);

    std::filesystem::remove(filename);
}
