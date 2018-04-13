cmake_minimum_required(VERSION 3.9)

project(Cache_Replacement)

set(CMAKE_CXX_FLAGS "-std=c++11 -pg -g -Wall")

include_directories(sim/inc)

file(GLOB_RECURSE sources sim/src/*.cpp)

add_library(Cache_Replacement_lib ${sources})

find_package(GTest)

if(GTEST_FOUND)
    enable_testing()
    add_executable(unit_tests test/test_main.cpp)
    target_link_libraries(unit_tests gtest gtest_main)
    target_link_libraries(unit_tests Cache_Replacement_lib)
    target_compile_definitions(unit_tests PUBLIC UNIT_TESTS)
    add_test(test_main unit_tests)
endif()