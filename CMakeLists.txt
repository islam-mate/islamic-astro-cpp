cmake_minimum_required(VERSION 3.15)
project(islammate-astro-cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Interface / Header-only library
add_library(islammate_astro INTERFACE)
target_include_directories(islammate_astro INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Test executable
add_executable(astro_test test/test_main.cpp)
target_link_libraries(astro_test PRIVATE islammate_astro)

enable_testing()
add_test(NAME AstroTest COMMAND astro_test)
