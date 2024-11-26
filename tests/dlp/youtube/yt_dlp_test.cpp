#include <gtest/gtest.h>
#include "../src/dlp/youtube/yt-dlp.h"
#include "../include/spotify-dlp.h"

using namespace std;

class YtDLPTest : public testing::Test
{
protected:
    YtDLP dlp;
    inline bool file_exists(const string &name)
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
    EXPECT_NE(result.out.find("Hello World"), string::npos);
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

    size_t newlines = count(result.out.begin(), result.out.end(), '\n');
    EXPECT_GT(newlines, 1);
}

TEST_F(YtDLPTest, TestDownloadSuccess)
{
    auto config = dlp_create_default_download_config();
    const string test_url = "https://www.youtube.com/watch?v=lsRqIwS9Iho";

    EXPECT_NO_THROW(dlp.download(config, test_url));
}

TEST_F(YtDLPTest, TestDownloadWithCustomConfig)
{
    auto config = dlp_create_default_download_config();
    config.download_file_type = MP3;
    config.retries = 5;
    config.audio_quality = 0;
    config.output = "%(title)s.%(ext)s";

    const string test_url = "https://www.youtube.com/watch?v=lsRqIwS9Iho";

    // TODO: Verify command construction with custom config
    EXPECT_NO_THROW(dlp.download(config, test_url));
}

TEST_F(YtDLPTest, TestDownloadGenericError)
{
    auto config = dlp_create_default_download_config();
    const string test_url = "https://www.youtube.com/watch?v=ThisIsAnInvalidVideoId123456789";

    try
    {
        dlp.download(config, test_url);
        FAIL() << "Expected runtime_error";
    }
    catch (const runtime_error &e)
    {
        SUCCEED();
    }
}