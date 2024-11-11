cmake_minimum_required(VERSION 3.15)
project(spotify_dlp)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(CURL REQUIRED)
set(CURL_USE_STATIC_LIBS ON) 
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(${PROJECT_NAME} src/main.cpp src/api/api.cpp src/utils/curl_utils.cpp)

FetchContent_Declare(
    nlohmann_json 
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

set(JSON_BuildTests OFF CACHE INTERNAL "")
set(BUILD_SHARED_LIBS OFF)

target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl nlohmann_json::nlohmann_json)
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CURL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)