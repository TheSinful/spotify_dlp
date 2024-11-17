#include <gtest/gtest.h>
#include "../src/dlp/youtube/yt-dlp.h"

class YtDLPTest_TestWindowsExtraction_Test : public testing::Test {
protected:
    YtDLP dlp;
};

TEST_F(YtDLPTest_TestWindowsExtraction_Test, TestWindowsExtraction) {
    dlp.extract_yt_dlp_windows();
    // Add your test assertions here
}