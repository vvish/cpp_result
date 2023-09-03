cmake_policy(SET CMP0048 NEW)

cmake_minimum_required(VERSION 3.9)

project(cpp_result VERSION 0.1)

include_directories(include)

add_subdirectory(test)
add_subdirectory(examples)

enable_testing()
