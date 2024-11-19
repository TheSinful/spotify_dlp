add_executable(${PROJECT_NAME}_test
    api_test.cpp     
    yt_dlp_test.cpp 
    log_test.cpp 
    youtube_search_builder_test.cpp
)

target_compile_definitions(${PROJECT_NAME}_test PRIVATE BUILD_TEST)

target_link_libraries(${PROJECT_NAME}_test PRIVATE
    ${PROJECT_NAME}_lib
    GTest::gtest_main
    CURL::libcurl 
)

target_include_directories(${PROJECT_NAME}_test PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}/include
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test
    PROPERTIES
        DISCOVERY_MODE PRE_TEST
        TEST_PREFIX "spotify_dlp."
    DISCOVERY_TIMEOUT 10
)