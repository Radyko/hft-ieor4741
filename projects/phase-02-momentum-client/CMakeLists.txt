cmake_minimum_required(VERSION 3.16)
project(phase_02_momentum CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(hft_server
        hft_server.cpp
)
target_include_directories(hft_server PRIVATE include)

add_executable(hft_client
        hft_client.cpp
)
target_include_directories(hft_client PRIVATE include)
