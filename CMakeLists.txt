cmake_minimum_required(VERSION 3.21)
project(ginga VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)

add_executable(ginga src/main.cpp)
target_link_libraries(ginga PRIVATE Qt6::Widgets)
if(MSVC)
    target_compile_options(ginga PRIVATE /W4 /utf-8)
else()
    target_compile_options(ginga PRIVATE -Wall -Wextra -Wpedantic)
endif()
