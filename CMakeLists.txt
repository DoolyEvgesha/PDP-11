cmake_minimum_required(VERSION 3.8)
project(PDP_11)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c)
add_executable(PDP_11 ${SOURCE_FILES})