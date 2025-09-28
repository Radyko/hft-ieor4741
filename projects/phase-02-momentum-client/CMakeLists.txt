add_executable(hft_server
        src/hft_server.cpp
)
target_include_directories(hft_server PRIVATE include)

add_executable(hft_client
        src/hft_client.cpp
)
target_include_directories(hft_client PRIVATE include)
