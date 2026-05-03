cmake_minimum_required(VERSION 3.20)
project(DM_Player LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Only include the files we actually need
file(GLOB SOURCES 
    "*.cpp"
    "Gui/*.cpp"
    "Searching/*.cpp"
)

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
    target_link_libraries(DM_Player PRIVATE d3d11 dxgi d3dcompiler dwmapi)
endif()

add_custom_command(TARGET DM_Player POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:DM_Player>/Assets
)