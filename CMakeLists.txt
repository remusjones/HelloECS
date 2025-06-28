cmake_minimum_required(VERSION 3.31)
project(HelloECS)

set(CMAKE_CXX_STANDARD 20)

add_executable(HelloECS main.cpp
        ECS/HelloECS.cpp
        ECS/HelloECS.h
        CoreTypes.h
)

target_include_directories(HelloECS PRIVATE ${CMAKE_SOURCE_DIR})
