cmake_minimum_required(VERSION 3.20)
project(tfm VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG        v6.1.9
    GIT_SHALLOW    TRUE
)

set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(ftxui)

file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.c")

add_executable(tfm ${SOURCES})

target_include_directories(tfm PRIVATE include)

target_link_libraries(tfm PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
)

target_compile_options(tfm PRIVATE
    $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic>
)
