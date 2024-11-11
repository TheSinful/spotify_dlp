add_executable(
    spotify_dlp_test
    api/api_test.cpp
)

target_link_libraries(
    spotify_dlp_test
    GTest::gtest_main
    spotify_dlp_core  
)

include(GoogleTest)
gtest_discover_tests(spotify_dlp_test)