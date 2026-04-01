cmake_minimum_required(VERSION 3.6)
project(VM)

set(CMAKE_CXX_STANDARD 23)

add_executable(VM
        src/main.cpp
        src/vm.cpp
)

target_include_directories(VM PRIVATE include)
