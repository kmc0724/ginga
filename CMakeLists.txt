cmake_minimum_required(VERSION 3.21)
project(ginga VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

add_library(ginga_core STATIC
    src/core/terminal_cell.h
    src/core/terminal_screen.h
    src/core/terminal_screen.cpp
    src/core/utf8_decoder.h
    src/core/utf8_decoder.cpp
)

target_compile_features(ginga_core PUBLIC cxx_std_20)

target_include_directories(ginga_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/core"
)

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

target_link_libraries(ginga PRIVATE
    Qt6::Widgets
    ginga_core
)

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
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(CTest)

if(BUILD_TESTING)
    add_executable(terminal_screen_test
        tests/terminal_screen_test.cpp
    )

    target_link_libraries(terminal_screen_test PRIVATE
        ginga_core
    )

    add_test(
        NAME terminal_screen_test
        COMMAND terminal_screen_test
    )

    add_executable(utf8_decoder_test
        tests/utf8_decoder_test.cpp
    )

    target_link_libraries(utf8_decoder_test PRIVATE
        ginga_core
    )

    add_test(
        NAME utf8_decoder_test
        COMMAND utf8_decoder_test
    )
endif()

if(MSVC)
    target_compile_options(terminal_screen_test PRIVATE /W4 /utf-8)
    target_compile_options(utf8_decoder_test PRIVATE /W4 /utf-8)
else()
    target_compile_options(terminal_screen_test PRIVATE
        -Wall -Wextra -Wpedantic
    )
    target_compile_options(utf8_decoder_test PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()
