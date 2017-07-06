cmake_minimum_required(VERSION 3.7)
project(xvm)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c Script.c Script.h Constants.h Stack.c Stack.h)
add_executable(xvm1 ${SOURCE_FILES})