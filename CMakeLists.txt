cmake_minimum_required(VERSION 3.15)
project(spotify_dlp)

set(CMAKE_CXX_STADARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(${PROJECT_NAME} src/main.cpp)