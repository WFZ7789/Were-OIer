cmake_minimum_required(VERSION 3.20)

project(WereOIer VERSION 0.3.0 LANGUAGES CXX)

option(WERE_OIER_BUILD_TESTS "Build Were-OIer tests" ON)

add_library(were_oier_core
    src/game.cpp
    src/input.cpp
)
target_include_directories(were_oier_core PUBLIC include)
target_compile_features(were_oier_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(were_oier_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(were_oier_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

add_executable(were_oier src/main.cpp)
target_link_libraries(were_oier PRIVATE were_oier_core)

if(WERE_OIER_BUILD_TESTS)
    enable_testing()
    add_executable(were_oier_tests tests/test_main.cpp)
    target_link_libraries(were_oier_tests PRIVATE were_oier_core)
    add_test(NAME were_oier_tests COMMAND were_oier_tests)
endif()

