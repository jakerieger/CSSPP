cmake_minimum_required(VERSION 3.29)
project(CSS++)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(Test main.cpp
        CSS.h
        Panic.h)
