find_package(SDL2 REQUIRED)

add_library(Frontend STATIC
        frontend.cpp
        frontend.h widgets/sequencer.cpp widgets/sequencer.h)

target_include_directories(Frontend PRIVATE
        ${SDL2_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(modules)

target_link_libraries(Frontend ImGui ${SDL2_LIBRARY})