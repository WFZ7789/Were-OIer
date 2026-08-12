cmake_minimum_required(VERSION 3.24)

project(WereOIer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(WERE_OIER_ENABLE_SANITIZERS "Enable address and undefined behavior sanitizers" OFF)

add_library(were_oier_core STATIC
  src/core/game.cpp
  src/core/ai.cpp
)

target_include_directories(were_oier_core
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(were_oier_core PUBLIC cxx_std_23)

if(MSVC)
  target_compile_options(were_oier_core PRIVATE /W4 /permissive-)
else()
  target_compile_options(were_oier_core PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

if(WERE_OIER_ENABLE_SANITIZERS AND NOT MSVC)
  target_compile_options(were_oier_core PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(were_oier_core PUBLIC -fsanitize=address,undefined)
endif()

add_executable(were_oier src/cli/main.cpp)
target_link_libraries(were_oier PRIVATE were_oier_core)

if(MSVC)
  target_compile_options(were_oier PRIVATE /W4 /permissive-)
else()
  target_compile_options(were_oier PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
endif()

include(CTest)
if(BUILD_TESTING)
  add_executable(were_oier_tests tests/test_core.cpp)
  target_link_libraries(were_oier_tests PRIVATE were_oier_core)

  if(MSVC)
    target_compile_options(were_oier_tests PRIVATE /W4 /permissive-)
  else()
    target_compile_options(were_oier_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  endif()

  add_test(NAME were_oier_tests COMMAND were_oier_tests)
endif()
