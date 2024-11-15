cmake_minimum_required(VERSION 3.15)
project(spotify_dlp)

include(CTest)
enable_testing()

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find CURL
find_package(CURL REQUIRED)
set(CURL_USE_STATIC_LIBS ON)
set(CMAKE_EXE_LINKER_FLAGS "-static")

# Add nlohmann/json
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# Add Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add spdlog 
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# Fetch fmt library
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2 
)
FetchContent_MakeAvailable(fmt)

# Add source files
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/api/api.cpp 
    src/api/metadata.cpp
    src/utils/curl_utils.cpp
)

set(BUILD_SHARED_LIBS OFF)


target_link_libraries(${PROJECT_NAME} PRIVATE 
    CURL::libcurl 
    nlohmann_json::nlohmann_json
    GTest::gtest_main
    spdlog::spdlog
    fmt::fmt
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CURL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)

# Add tests
add_subdirectory(tests)