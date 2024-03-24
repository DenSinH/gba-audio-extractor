cmake_minimum_required(VERSION 3.21)
project(GbaMusicExtractor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include_directories(src/include)
include_directories(src)

add_executable(GbaMusicExtractor
        src/static.cpp
        src/main.cpp)

add_subdirectory(src/extractor)
add_subdirectory(src/frontend)

target_link_libraries(GbaMusicExtractor Extractor Frontend)

if(WIN32)
    set(SDL_DLL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/files/SDL2.dll")
    add_custom_command(TARGET GbaMusicExtractor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL_DLL_PATH}"
        "${CMAKE_BINARY_DIR}"
        COMMENT "Copying SDL2.dll to build directory"
    )
endif()
