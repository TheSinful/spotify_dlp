cmake_minimum_required(VERSION 3.15)
project(spotify_dlp)

include(FetchContent) 

option(BUILD_TESTS "Build tests" OFF) 

if(BUILD_TESTS) 
    include(CTest)
    enable_testing()
    add_compile_definitions(BUILD_TEST)
    
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    
    include_directories(${gtest_SOURCE_DIR}/include)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find CURL
find_package(CURL REQUIRED)
set(CURL_USE_STATIC_LIBS ON)

# Add nlohmann/json
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.2 
)
FetchContent_MakeAvailable(fmt)

if(WIN32)
    add_compile_definitions(PLATFORM_WINDOWS)
    set(PLATFORM_BINARY_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/dlp/youtube/data/yt-dlp-data-windows.cpp")
else()
    add_compile_definitions(PLATFORM_LINUX)
    set(PLATFORM_BINARY_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/dlp/youtube/data/yt-dlp-data-linux.cpp")
endif()

add_library(yt-dlp-data STATIC ${PLATFORM_BINARY_SOURCE})

target_link_libraries(yt-dlp-data PRIVATE 
    CURL::libcurl 
    nlohmann_json::nlohmann_json
)

add_library(${PROJECT_NAME}_lib
    src/dlp/spotify/api.cpp
    src/dlp/spotify/metadata.cpp
    src/utils/curl_utils.cpp
    src/dlp/youtube/yt-dlp.cpp
)

target_compile_definitions(${PROJECT_NAME}_lib PUBLIC BUILD_TEST)
target_include_directories(${PROJECT_NAME}_lib PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
    ${CURL_INCLUDE_DIRS}
    $<$<BOOL:${BUILD_TESTS}>:${gtest_SOURCE_DIR}/include>
)

target_link_libraries(${PROJECT_NAME}_lib PUBLIC
    CURL::libcurl
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    fmt::fmt
    yt-dlp-data
)

# Add source files
add_executable(${PROJECT_NAME} 
    src/main.cpp 
    src/dlp/spotify/api.cpp 
    src/dlp/spotify/metadata.cpp
    src/utils/curl_utils.cpp
)

if(MSVC)
    target_compile_options(yt-dlp-data PRIVATE "/MP1" "/bigobj")
else()
    target_compile_options(yt-dlp-data PRIVATE "-Wa,-mbig-obj")
endif()

set(BUILD_SHARED_LIBS OFF)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    CURL::libcurl 
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    fmt::fmt
    yt-dlp-data
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CURL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)

if(BUILD_TESTS) 
    add_subdirectory(tests)
endif()