find_package(SDL2 REQUIRED)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

add_library(ImGui STATIC
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
        imgui-filebrowser/imfilebrowser.h
        sequencer/ImSequencer.cpp
        sequencer/ImSequencer.h
        implot/implot_internal.h
        implot/implot_items.cpp
        implot/implot.h
        implot/implot.cpp
)

target_include_directories(ImGui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${SDL2_INCLUDE_DIR}
)

add_library(Glad STATIC
        glad/glad.c
        glad/glad.h)

target_include_directories(Glad PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(ImGui Glad)