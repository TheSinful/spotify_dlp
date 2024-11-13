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

TEST_F(SpotifyAPITest, SerializeArtistResponse)
{
    // grabbed from spotify docs (edited slightly to be 100% sure we read the right fields)
    std::string test_response = R"({
"external_urls": {
    "spotify": "spotify_url", 
    "youtube": "youtube_url"
},
"followers": {
    "href": "string",
    "total": 0
},
"genres": ["Prog rock", "Grunge"],
"href": "string",
"id": "test_id",
"images": [
    {
    "url": "https://i.scdn.co/image/ab67616d00001e02ff9ca10b55ce82ae553c8228",
    "height": 300,
    "width": 300
    }
],
"name": "test_name",
"popularity": 0,
"type": "artist",
"uri": "test_uri"
})";

    auto to_json = nlohmann::json::parse(test_response);
    Artist serialized = Artist::serialize(to_json);

    Artist manually_serialized;
    manually_serialized.external_urls = {"spotify_url", "youtube_url"};
    manually_serialized.id = "test_id";
    manually_serialized.name = "test_name";
    manually_serialized.uri = "test_uri";

    ASSERT_EQ(serialized, manually_serialized);
}

TEST_F(SpotifyAPITest, SerializeTrackResponse)
{
    std::string test_response = R"({
  "track_number": 5,
  "external_urls": {
    "spotify": "test_external_url"
  },
  "id": "test_track_id",
  "name": "test_track_name",
  "album": {
    "name": "test_album_name",
    "id": "test_album_id",
    "release_date": "1981-12",
    "images": [
      {
        "url": "test_album_cover",
        "height": 300,
        "width": 300
      }
    ]
  },
  "artists": [
    {
      "external_urls": {
        "spotify": "test_artist_url"
      },
      "id": "test_artist_id",
      "name": "test_artist_name",
      "uri": "test_artist_uri"
    }
  ],
  "uri": "test_track_uri"
})";

    auto to_json = nlohmann::json::parse(test_response);
    TrackMetadata serialized = TrackMetadata::serialize(to_json);

    Artist artist;
    artist.external_urls = {"test_artist_url"};
    artist.id = "test_artist_id";
    artist.name = "test_artist_name";
    artist.uri = "test_artist_uri";

    TrackMetadata expected;
    expected.name = "test_track_name";
    expected.artists = {artist};
    expected.id = "test_track_id";
    expected.album_name = "test_album_name";
    expected.album_id = "test_album_id";
    expected.external_urls = {"test_external_url"};
    expected.track_number = 5;

    ASSERT_EQ(serialized, expected);
}