#include <gtest/gtest.h>
#include "../src/spotify/api.h"

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

    void ValidateTrackMetadata(const TrackMetadata &track)
    {
        EXPECT_FALSE(track.name.empty());
        EXPECT_FALSE(track.id.empty());
        EXPECT_FALSE(track.artists.empty());
        EXPECT_FALSE(track.external_urls.empty());
        EXPECT_GT(track.track_number, 0);

        // Validate album info
        EXPECT_TRUE(track.album_name.has_value());
        EXPECT_TRUE(track.album_id.has_value());
        EXPECT_FALSE(track.album_name->empty());
        EXPECT_FALSE(track.album_id->empty());

        // Validate first artist
        const auto &artist = track.artists[0];
        EXPECT_FALSE(artist.name.empty());
        EXPECT_FALSE(artist.id.empty());
        EXPECT_FALSE(artist.external_urls.empty());
    }

    void ValidateAlbumMetadata(const AlbumMetadata &album)
    {
        EXPECT_FALSE(album.name.empty());
        EXPECT_FALSE(album.id.empty());
        EXPECT_FALSE(album.release_date.empty());
        EXPECT_GT(album.total_tracks, 0);
        EXPECT_FALSE(album.artists.empty());
        EXPECT_FALSE(album.tracks.empty());

        // Validate first track
        ValidateTrackMetadata(album.tracks[0]);
    }

    void ValidatePlaylistMetadata(const PlaylistMetadata &playlist)
    {
        EXPECT_FALSE(playlist.name.empty());
        EXPECT_FALSE(playlist.id.empty());
        EXPECT_GT(playlist.total_tracks, 0);
        EXPECT_FALSE(playlist.tracks.empty());

        // Validate first track
        ValidateTrackMetadata(playlist.tracks[0]);
    }
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
    auto [type, id] = api.extract_type_and_id("track/123456");
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
    api.fetch_oauth_token();
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
TEST_F(SpotifyAPITest, SerializeAlbumResponse)
{
    std::string test_response = R"({
        "name": "test_name",
        "id": "test_id", 
        "total_tracks": 1,
        "release_date": "1981-12",
        "artists": [{
            "external_urls": {"spotify": "test_url"},
            "id": "test_artist_id",
            "name": "test_artist_name",
            "uri": "test_artist_uri"
        }],
        "tracks": {
            "items": [{
                "external_urls": {"spotify": "test_url"},
                "id": "track_id",
                "name": "track_name",
                "track_number": 1,
                "artists": [{
                    "external_urls": {"spotify": "test_url"},
                    "id": "test_artist_id", 
                    "name": "test_artist_name",
                    "uri": "test_artist_uri"
                }],
                "uri": "test_track_uri"
            }]
        }
    })";

    auto to_json = nlohmann::json::parse(test_response);
    AlbumMetadata serialized = AlbumMetadata::serialize(to_json);

    Artist manual_artist;
    manual_artist.name = "test_artist_name";
    manual_artist.id = "test_artist_id";
    manual_artist.external_urls = {"test_url"};
    manual_artist.uri = "test_artist_uri";

    TrackMetadata track;
    track.name = "track_name";
    track.id = "track_id";
    track.track_number = 1;
    track.external_urls = {"test_url"};
    track.artists = {manual_artist};
    track.album_name = "test_name";
    track.album_id = "test_id";

    AlbumMetadata manual;
    manual.name = "test_name";
    manual.id = "test_id";
    manual.artists = {manual_artist};
    manual.release_date = "1981-12";
    manual.total_tracks = 1;
    manual.tracks = {track};

    ASSERT_EQ(serialized, manual);
}
TEST_F(SpotifyAPITest, SerializePlaylistResponse)
{
    std::string test_response = R"({
        "name": "test_playlist",
        "id": "test_playlist_id",
        "tracks": {
            "total": 1,
            "items": [{
                "track": {
                    "name": "test_track_name",
                    "id": "test_track_id",
                    "track_number": 1,
                    "external_urls": {
                        "spotify": "test_track_url"
                    },
                    "album": {
                        "name": "test_album_name",
                        "id": "test_album_id"
                    },
                    "artists": [{
                        "external_urls": {
                            "spotify": "test_artist_url"
                        },
                        "id": "test_artist_id",
                        "name": "test_artist_name",
                        "uri": "test_artist_uri"
                    }]
                }
            }]
        }
    })";

    auto to_json = nlohmann::json::parse(test_response);
    PlaylistMetadata serialized = PlaylistMetadata::serialize(to_json);

    Artist manual_artist;
    manual_artist.name = "test_artist_name";
    manual_artist.id = "test_artist_id";
    manual_artist.external_urls = {"test_artist_url"};
    manual_artist.uri = "test_artist_uri";

    TrackMetadata track;
    track.name = "test_track_name";
    track.id = "test_track_id";
    track.track_number = 1;
    track.external_urls = {"test_track_url"};
    track.artists = {manual_artist};
    track.album_name = "test_album_name";
    track.album_id = "test_album_id";

    PlaylistMetadata expected;
    expected.name = "test_playlist";
    expected.id = "test_playlist_id";
    expected.total_tracks = 1;
    expected.tracks = {track};

    ASSERT_EQ(serialized, expected);
}
TEST_F(SpotifyAPITest, FetchTrackMetadataReturnsCorrectData)
{
    SpotifyAPI api(this->client_id, this->client_secret);
    api.item_id = "5DQiTQrSoYOZxm5oj3lR4l";

    TrackMetadata result = api.fetch_track_metadata();
    ValidateTrackMetadata(result);
}
TEST_F(SpotifyAPITest, FetchAlbumMetadataReturnsCorrectData)
{
    SpotifyAPI api(this->client_id, this->client_secret);
    api.item_id = "55S2SOsWCYekWJtJ8LwVqV";

    AlbumMetadata result = api.fetch_album_metadata();
    ValidateAlbumMetadata(result);
}
TEST_F(SpotifyAPITest, FetchPlaylistMetadataReturnsCorrectData)
{
    SpotifyAPI api(this->client_id, this->client_secret);
    api.item_id = "7464s7OIoUO0k23f1uxzLL";

    PlaylistMetadata result = api.fetch_playlist_metadata();
    ValidatePlaylistMetadata(result);
}