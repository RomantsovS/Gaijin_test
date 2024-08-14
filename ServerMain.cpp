#include "Logger.h"
#include "Server.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) {
    Logger::enabled = true;
    Database db;
    const std::filesystem::path filename = "config.txt";
    DatabaseReader::ReadFromFile(db, filename);
    Core core(db);

    core.setDBWriter(std::move(std::make_unique<DatabaseWriter>(db, filename)));

    Server server(7899, core, 4);
    server.Run();

    return 0;
}
