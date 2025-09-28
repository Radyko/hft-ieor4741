cmake_minimum_required(VERSION 3.16)
project(hft_ieor4741 CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Auto-include each immediate subfolder of projects/
file(GLOB children RELATIVE "${CMAKE_SOURCE_DIR}/projects" "${CMAKE_SOURCE_DIR}/projects/*")
foreach(child ${children})
    if(IS_DIRECTORY "${CMAKE_SOURCE_DIR}/projects/${child}")
        add_subdirectory("projects/${child}")
    endif()
endforeach()
