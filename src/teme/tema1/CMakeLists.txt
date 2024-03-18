cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 17)
project(tema1 VERSION 1.0 LANGUAGES CXX)

find_package(libpqxx CONFIG REQUIRED)
find_package(spdlog REQUIRED)
find_package(cryptopp REQUIRED)

add_executable(tema1 src/bank.h src/database.h src/main.cpp)
target_link_libraries(tema1 PRIVATE libpqxx::pqxx spdlog::spdlog_header_only cryptopp::cryptopp)