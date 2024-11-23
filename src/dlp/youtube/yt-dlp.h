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
    FRIEND_TEST(YtDLPTest, TestWindowsExtraction);
    FRIEND_TEST(YtDLPTest, TestExecuteCommand);
    FRIEND_TEST(YtDLPTest, TestInvalidCommand);
    FRIEND_TEST(YtDLPTest, TestStdErrOutput);
    FRIEND_TEST(YtDLPTest, TestMultilineOutput);
    FRIEND_TEST(YtDLPTest, TestDownloadSuccess);
    FRIEND_TEST(YtDLPTest, TestDownloadWithCustomConfig);
    FRIEND_TEST(YtDLPTest, TestDownloadGenericError);
#endif

public:
    YtDLP();
    void download(DownloadConfig config, const std::string &url);

    std::string get_path() const;

private:
    CommandResult execute_command(const std::string &command);
    std::string get_download_file_type();
    bool command_return_ok(YtDLPExitCodes code);
    void extract_yt_dlp_windows();
    void extract_yt_dlp_linux();
    void get_temp_path();

    std::filesystem::path yt_dlp_temp_path;
    std::string program_name;
    DownloadConfig config;
};

#endif