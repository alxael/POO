#include <iostream>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <curses.h>

#include "exception.h"
#include "validation.h"
#include "bank.h"
#include "database.h"
#include "interface.h"

using namespace std;
using namespace spdlog;
using namespace interface;

int main(int argc, char *argv[])
{
    // set up logging
    auto logger = basic_logger_mt("logger", "logs/main.log");
    set_default_logger(logger);
    flush_on(level::info);

    // get database name
    string databaseName = "poo";
    if (argc > 1)
        databaseName = string(argv[1]);

    CLI cli("postgresql://postgres:postgres@localhost:5432/" + databaseName, "scripts/initializeDatabase.sql");
    cli.start();
    return 0;
}