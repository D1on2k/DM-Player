cmake_minimum_required(VERSION 3.20)
project(DM_Player LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB SOURCES "*.cpp" "Gui/*.cpp" "Searching/*.cpp" "PathTab4/*.cpp")

file(GLOB IMGUI_CORE "ImGui/*.cpp")
file(GLOB IMGUI_BACKEND "ImGui/backends/imgui_impl_win32.cpp" "ImGui/backends/imgui_impl_dx11.cpp")

list(APPEND SOURCES ${IMGUI_CORE})
list(APPEND SOURCES ${IMGUI_BACKEND})

add_executable(DM_Player ${SOURCES})

target_include_directories(DM_Player PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/ImGui
    ${CMAKE_SOURCE_DIR}/ImGui/backends
    ${CMAKE_SOURCE_DIR}/Includes
)

if(WIN32)

    target_link_libraries(DM_Player PRIVATE d3d11 dxgi d3dcompiler_47 dwmapi gdi32 user32 ole32 shell32 uuid)

    target_link_options(DM_Player PRIVATE -mwindows)

    set(dlls
        C:/msys64/ucrt64/bin/libstdc++-6.dll
        C:/msys64/ucrt64/bin/libgcc_s_seh-1.dll
        C:/msys64/ucrt64/bin/libwinpthread-1.dll
    )

    foreach(d ${dlls})
        add_custom_command(TARGET DM_Player POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${d} $<TARGET_FILE_DIR:DM_Player>
        )
    endforeach()

endif()

add_custom_command(TARGET DM_Player POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:DM_Player>/Assets
)
