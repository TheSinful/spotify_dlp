#include <gtest/gtest.h>
#include "../src/api/api.h"

class SpotifyAPITest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        client_id = "c620ce69c5ec439f9c519c30dfb7b18c";
        client_secret = "b1dc312867a34e9ea764173664b33499";
    }

    std::string client_id;
    std::string client_secret;
};

TEST_F(SpotifyAPITest, ValidateAndCleanUrlRemovesQueryParameters)
{
    SpotifyAPI api(this->client_id, this->client_secret);
    std::string url = "https://open.spotify.com/track/123456?si=abcdef";
    std::string cleaned = api.validate_and_clean_url(url);
    EXPECT_EQ(cleaned, "track/123456");
}

TEST_F(SpotifyAPITest, ValidateAndCleanUrlHandlesShortUrl)
{
    SpotifyAPI api(this->client_id, this->client_secret);
    std::string url = "track/123456";
    std::string cleaned = api.validate_and_clean_url(url);
    EXPECT_EQ(cleaned, "track/123456");
}

TEST_F(SpotifyAPITest, ValidateAndCleanUrlThrowsOnSpotifyUri)
{
    SpotifyAPI api(this->client_id, this->client_secret);
    std::string url = "spotify:track:123456";
    EXPECT_THROW(api.validate_and_clean_url(url), std::runtime_error);
}

TEST_F(SpotifyAPITest, SplitUrlParsesCorrectly)
{
    SpotifyAPI api(this->client_id, this->client_secret);
    auto [type, id] = api.split_url("track/123456");
    EXPECT_EQ(type, "track");
    EXPECT_EQ(id, "123456");
}

TEST_F(SpotifyAPITest, DetermineContentTypeReturnsCorrectType)
{
    SpotifyAPI api(this->client_id, this->client_secret);
    EXPECT_EQ(api.determine_content_type("track"), DownloadType::Track);
    EXPECT_EQ(api.determine_content_type("album"), DownloadType::Album);
    EXPECT_EQ(api.determine_content_type("playlist"), DownloadType::Playlist);
    EXPECT_THROW(api.determine_content_type("invalid"), std::runtime_error);
}

TEST_F(SpotifyAPITest, ValidateFetchToken)
{
    SpotifyAPI api(this->client_id, this->client_secret);
    api.fetch_token();
    EXPECT_FALSE(api.token.empty());
}