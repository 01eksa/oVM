cmake_minimum_required(VERSION 4.1)
project(VM)

set(CMAKE_CXX_STANDARD 23)

add_executable(VM
        src/main.cpp
        include/loader.h
)

target_include_directories(VM PRIVATE include)
