#pragma once
#ifndef YT_DLP_H
#define YT_DLP_H

#ifdef BUILD_TEST
#include <gtest/gtest.h>
#endif

#include <filesystem>
#include "../../../include/spotify-dlp.h"

enum YtDLPExitCodes
{
    Success = 0,
    UpdateRequired = 100,
    CancelledByMaxDownloads = 101,
    UserOptionsError = 2,
    GenericError = 1,
};

struct CommandResult
{
    int cmd_exit_code;
    YtDLPExitCodes ytdlp_exit_code;
    std::string out;
    std::string err;
};

class YtDLP
{
#ifdef BUILD_TEST
    FRIEND_TEST(YtDLPTest_TestWindowsExtraction_Test, TestWindowsExtraction);
#endif

public:
    YtDLP();
    void download(DownloadConfig config);

    std::string get_path() const;

private:
    std::string get_download_file_type();
    void extract_yt_dlp_windows();
    void extract_yt_dlp_linux();
    void get_temp_path();
    CommandResult execute_command(const std::string &command);
    bool command_return_ok(YtDLPExitCodes code);

    std::filesystem::path yt_dlp_temp_path;
    std::string program_name;

    DownloadConfig config;
};

#endif