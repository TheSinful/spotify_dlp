add_executable(${PROJECT_NAME}_test
    api_test.cpp     
    log_test.cpp 
    ${CMAKE_SOURCE_DIR}/src/api/api.cpp
    ${CMAKE_SOURCE_DIR}/src/api/metadata.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/curl_utils.cpp
)

target_link_libraries(${PROJECT_NAME}_test PRIVATE
    CURL::libcurl
    nlohmann_json::nlohmann_json
    GTest::gtest_main
    spdlog::spdlog 
)

target_include_directories(${PROJECT_NAME}_test PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test
    PROPERTIES
        DISCOVERY_MODE PRE_TEST
        TEST_PREFIX "spotify_dlp."
    DISCOVERY_TIMEOUT 10
)