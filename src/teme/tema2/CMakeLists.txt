cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 23)
project(tema2 VERSION 1.0 LANGUAGES CXX)

find_package(libpqxx CONFIG REQUIRED)
find_package(spdlog REQUIRED)
find_package(cryptopp REQUIRED)

add_executable(tema2 src/bank.h src/database.h src/interface.h src/main.cpp)
target_link_libraries(tema2 PRIVATE libpqxx::pqxx spdlog::spdlog_header_only cryptopp::cryptopp)