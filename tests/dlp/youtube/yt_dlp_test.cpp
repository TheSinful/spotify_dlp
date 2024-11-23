#include <gtest/gtest.h>
#include "../src/dlp/youtube/yt-dlp.h"

class YtDLPTest : public testing::Test
{
protected:
    YtDLP dlp;
    inline bool file_exists(const std::string &name)
    {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }
};

// idk how to test for linux because we check in compiliation if we are on linux (im not) there's probably someway to but if
// its an issue it'll get brought up in the future
TEST_F(YtDLPTest, TestWindowsExtraction)
{
    dlp.extract_yt_dlp_windows();

    ASSERT_TRUE(file_exists(dlp.yt_dlp_temp_path.string()));
}

TEST_F(YtDLPTest, TestExecuteCommand)
{
#ifdef _WIN32
    auto result = dlp.execute_command("echo Hello World");
#else
    auto result = dlp.execute_command("/bin/echo Hello World");
#endif

    EXPECT_EQ(result.cmd_exit_code, 0);
    EXPECT_EQ(result.ytdlp_exit_code, YtDLPExitCodes::Success);
    EXPECT_TRUE(result.err.empty());
    EXPECT_NE(result.out.find("Hello World"), std::string::npos);
}

TEST_F(YtDLPTest, TestInvalidCommand)
{
    auto result = dlp.execute_command("invalid_command_that_doesnt_exist");

    EXPECT_NE(result.cmd_exit_code, 0);
    EXPECT_FALSE(result.err.empty());
}

TEST_F(YtDLPTest, TestStdErrOutput)
{
#ifdef _WIN32
    auto result = dlp.execute_command("dir /invalid_flag");
#else
    auto result = dlp.execute_command("ls --invalid_flag");
#endif

    EXPECT_NE(result.cmd_exit_code, 0);
    EXPECT_FALSE(result.err.empty());
}

TEST_F(YtDLPTest, TestMultilineOutput)
{
#ifdef _WIN32
    auto result = dlp.execute_command("dir");
#else
    auto result = dlp.execute_command("ls -la");
#endif

    EXPECT_EQ(result.cmd_exit_code, 0);
    EXPECT_EQ(result.ytdlp_exit_code, YtDLPExitCodes::Success);
    EXPECT_TRUE(result.err.empty());

    size_t newlines = std::count(result.out.begin(), result.out.end(), '\n');
    EXPECT_GT(newlines, 1);
}