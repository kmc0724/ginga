cmake_minimum_required(VERSION 3.21)
project(ginga VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)


add_executable(ginga
    src/main.cpp
    src/gui/terminal_widget.h
    src/gui/terminal_widget.cpp
)

# Release: GUI application (no console)
# Debug: Console application (for logging)
set_target_properties(ginga PROPERTIES
    WIN32_EXECUTABLE "$<CONFIG:Release>"
)

target_link_libraries(ginga PRIVATE Qt6::Widgets)

# Copy Qt runtime DLLs after each build (Windows)
if(WIN32)
    add_custom_command(TARGET ginga POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:ginga>
            $<TARGET_FILE_DIR:ginga>
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:ginga>/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Qt6::QWindowsIntegrationPlugin>"
            "$<TARGET_FILE_DIR:ginga>/platforms/"
        COMMAND_EXPAND_LISTS
        COMMENT "Deploying Qt runtime and Windows platform plugin"
    )
endif()

if(MSVC)
    target_compile_options(ginga PRIVATE /W4 /utf-8)
else()
    target_compile_options(ginga PRIVATE -Wall -Wextra -Wpedantic)
endif()
