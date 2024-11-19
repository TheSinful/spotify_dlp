#include <gtest/gtest.h>
#include "../src/dlp/youtube/yt-dlp.h"

class YtDLPTest_TestWindowsExtraction_Test : public testing::Test
{
protected:
    YtDLP dlp;
    inline bool file_exists(const std::string &name)
    {
        struct stat buffer;
        return (stat(name.c_str(), &buffer) == 0);
    }
};

TEST_F(YtDLPTest_TestWindowsExtraction_Test, TestWindowsExtraction)
{
    dlp.extract_yt_dlp_windows();

    ASSERT_TRUE(file_exists(dlp.yt_dlp_temp_path.string()));
}

// idk how to test for linux because we check in compiliation if we are on linux (im not) there's probably someway to but if 
// its an issue it'll get brought up in the future 