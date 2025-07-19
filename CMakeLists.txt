cmake_minimum_required(VERSION 3.31)
project(HelloECS)

set(CMAKE_CXX_STANDARD 20)

add_library(HelloECS STATIC
        ECS/HelloECS.cpp
        ECS/HelloECS.h
        ECS/CoreTypes.h
)

target_include_directories(HelloECS PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(HelloECSApp main.cpp)
target_link_libraries(HelloECSApp HelloECS)