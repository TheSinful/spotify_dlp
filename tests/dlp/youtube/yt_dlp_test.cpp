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
    // ...remove existing test as it's been split into more specific tests above...
}

TEST_F(YtDLPTest, TestDownloadWithCustomConfig)
{
    auto config = dlp_create_default_config_with_download_path("D:\\coding\\spotify_dlp\\build\\Testing\\Temporary");
    config.download_file_type = MP3;
    config.retries = 5;
    config.audio_quality = 0;

    const string test_url = "https://www.youtube.com/watch?v=lsRqIwS9Iho";

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

TEST_F(YtDLPTest, TestGetRealOutputPath)
{
    CommandResult result;
    result.out = R"([download] Destination: temp.webm
[ExtractAudio] Destination: output.mp3
[download] 100% of 5.00MiB)";

    auto path = dlp.get_real_output_path(result);
    EXPECT_EQ(path.filename().string(), "output.mp3");
}

TEST_F(YtDLPTest, TestGetRealOutputPathMissing)
{
    CommandResult result;
    result.out = "[download] 100% of 5.00MiB";

    EXPECT_THROW(dlp.get_real_output_path(result), std::runtime_error);
}

TEST_F(YtDLPTest, TestDownloadReturnsPath)
{
    auto config = dlp_create_default_download_config();
    const string test_url = "https://www.youtube.com/watch?v=lsRqIwS9Iho";

    auto path = dlp.download(config, test_url);
    EXPECT_FALSE(path.empty());
    EXPECT_TRUE(filesystem::exists(path));
}

TEST_F(YtDLPTest, TestDownloadWithCustomOutputPath)
{
    auto config = dlp_create_default_config_with_download_path("test_output");
    const string test_url = "https://www.youtube.com/watch?v=lsRqIwS9Iho";

    auto path = dlp.download(config, test_url);
    EXPECT_FALSE(path.empty());

    // Test the filename properties
    EXPECT_EQ(path.stem().string(), "test_output");

    // Get the absolute paths for comparison
    auto current_path = std::filesystem::absolute(std::filesystem::current_path());
    auto parent_path = std::filesystem::absolute(path.parent_path());
    EXPECT_EQ(parent_path, current_path);
}

TEST_F(YtDLPTest, TestDownloadPathConfig)
{
    const string test_url = "https://www.youtube.com/watch?v=lsRqIwS9Iho";

    // Test with specific filename
    {
        auto config = dlp_create_default_config_with_download_path("test_file.mp3");
        auto path = dlp.download(config, test_url);
        EXPECT_EQ(path.stem().string(), "test_file");
        EXPECT_TRUE(filesystem::exists(path));
    }

    // Test with path without extension
    {
        auto config = dlp_create_default_config_with_download_path("test_file");
        auto path = dlp.download(config, test_url);
        EXPECT_EQ(path.stem().string(), "test_file");
        EXPECT_TRUE(filesystem::exists(path));
    }

    // Test with directory path (should fail)
    {
        auto config = dlp_create_default_config_with_download_path(".");
        EXPECT_EQ(config.error, ERROR_DIRECTORY_PATH_NOT_ALLOWED);
    }

    // Test with relative path
    {
        auto config = dlp_create_default_config_with_download_path("./downloads/test_file.mp3");
        filesystem::create_directories("downloads");
        auto path = dlp.download(config, test_url);
        EXPECT_EQ(path.parent_path().filename().string(), "downloads");
        EXPECT_EQ(path.stem().string(), "test_file");
        EXPECT_TRUE(filesystem::exists(path));
        filesystem::remove_all("downloads");
    }

    // Test with absolute path
    {
        filesystem::path abs_path = filesystem::absolute("test_absolute.mp3");
        auto config = dlp_create_default_config_with_download_path(abs_path.string().c_str());
        auto path = dlp.download(config, test_url);
        EXPECT_EQ(path.stem().string(), "test_absolute");
        EXPECT_EQ(path.parent_path(), abs_path.parent_path());
        EXPECT_TRUE(filesystem::exists(path));
    }
}