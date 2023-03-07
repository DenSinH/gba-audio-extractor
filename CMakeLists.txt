cmake_minimum_required(VERSION 3.21)
project(GbaMusicExtractor)

set(CMAKE_CXX_STANDARD 20)

include_directories(src/include)

add_executable(GbaMusicExtractor
        src/main.cpp src/include/gba.h src/include/bin.h)

add_subdirectory(src/extractor)

target_link_libraries(GbaMusicExtractor Extractor)
