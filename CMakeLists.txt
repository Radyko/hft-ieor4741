cmake_minimum_required(VERSION 3.16)
project(hft LANGUAGES CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Threads (for -pthread)
find_package(Threads REQUIRED)

# Client executable
add_executable(hft_client hft_client.cpp)
target_link_libraries(hft_client PRIVATE Threads::Threads)

# Server executable
add_executable(hft_server hft_server.cpp)
target_link_libraries(hft_server PRIVATE Threads::Threads)
