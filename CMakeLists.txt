cmake_minimum_required(VERSION 3.21)
project(GbaMusicExtractor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include_directories(src/include)

add_executable(GbaMusicExtractor
        src/main.cpp
        src/include/gba.h
        src/include/bin.h
        src/include/algorithm.h)

add_subdirectory(src/extractor)
add_subdirectory(src/frontend)

target_link_libraries(GbaMusicExtractor Extractor Frontend)
