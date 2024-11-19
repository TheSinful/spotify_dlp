#pragma once
#ifndef YT_DLP_H
#define YT_DLP_H

#ifdef BUILD_TEST
#include <gtest/gtest.h>
#endif

#include <filesystem>

class YtDLP
{
#ifdef BUILD_TEST
    FRIEND_TEST(YtDLPTest_TestWindowsExtraction_Test, TestWindowsExtraction);
#endif

public:
    YtDLP();

    std::string get_path() const;
private:
    void extract_yt_dlp_windows();
    void extract_yt_dlp_linux();
    void get_temp_path();

    std::filesystem::path yt_dlp_temp_path;
    std::string program_name;
};

#endif