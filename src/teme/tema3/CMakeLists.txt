cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)
project(tema3 VERSION 1.0 LANGUAGES CXX)

find_package(libpqxx CONFIG REQUIRED)
find_package(spdlog REQUIRED)
find_package(cryptopp REQUIRED)

add_executable(tema3
    src/validation.hpp
    src/exception.hpp
    src/bank.hpp
    src/query.hpp
    src/database.hpp
    src/interface.hpp
    src/main.cpp
)
target_link_libraries(tema3 PRIVATE libpqxx::pqxx spdlog::spdlog_header_only cryptopp::cryptopp)