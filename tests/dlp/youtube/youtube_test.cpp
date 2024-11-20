
#include <gtest/gtest.h>
#include <typeinfo>
#include <variant>
#include "../src/dlp/spotify/metadata.h"
#include "../src/dlp/spotify/api.h"
#include "../src/dlp/youtube/youtube.h"

class YoutubeTest : public ::testing::Test
{
protected:
    SpotifyAPI api;
    std::string api_key;

    YoutubeTest()
        : api("c620ce69c5ec439f9c519c30dfb7b18c", "b1dc312867a34e9ea764173664b33499"),
          api_key("AIzaSyBJavV8s_t99123wP1u3QUGknae4RvurDE")
    {
    }

    SearchResult create_test_result(const std::string &title,
                                    const std::string &artist,
                                    const std::string &id = "test_id",
                                    int duration = 0)
    {
        return SearchResult{
            .video_id = id,
            .title = title,
            .artist = artist,
            .duration_seconds = duration,
            .score = 0.0};
    }

    TrackMetadata create_test_track()
    {
        Artist artist{
            .external_urls = {"https://open.spotify.com/artist/1"},
            .id = "1",
            .name = "Test Artist",
            .uri = "spotify:artist:1"};

        return TrackMetadata{
            .name = "Test Track",
            .artists = {artist},
            .id = "track1",
            .album_name = "Test Album",
            .album_id = "album1",
            .external_urls = {"https://open.spotify.com/track/1"},
            .track_number = 1};
    }

    AlbumMetadata create_test_album()
    {
        Artist artist{
            .external_urls = {"https://open.spotify.com/artist/1"},
            .id = "1",
            .name = "Test Artist",
            .uri = "spotify:artist:1"};

        return AlbumMetadata{
            .name = "Test Album",
            .artists = {artist},
            .id = "album1",
            .release_date = "2023",
            .total_tracks = 1,
            .tracks = {create_test_track()}};
    }
};

TEST_F(YoutubeTest, CalculateMatchScoreTrackTest)
{
    TrackMetadata track = create_test_track();
    Youtube yt(track, api_key);

    // Perfect match
    auto perfect_result = create_test_result("Test Track", "Test Artist");
    EXPECT_NEAR(yt.calculate_match_score(perfect_result), 0.9, 0.1);

    // Partial match
    auto partial_result = create_test_result("Test Track remix", "Different Artist");
    EXPECT_GT(yt.calculate_match_score(partial_result), 0.3);
    EXPECT_LT(yt.calculate_match_score(partial_result), 0.7);

    // Poor match
    auto poor_result = create_test_result("Completely Different", "Random Artist");
    EXPECT_LT(yt.calculate_match_score(poor_result), 0.3);
}

TEST_F(YoutubeTest, CalculateMatchScoreAlbumTest)
{
    AlbumMetadata album = create_test_album();
    Youtube yt(album, api_key);

    // Perfect album match
    auto perfect_result = create_test_result("Test Album", "Test Artist");
    EXPECT_NEAR(yt.calculate_match_score(perfect_result), 1.0, 0.1);

    // Partial match
    auto partial_result = create_test_result("Test Album songs", "Different Artist");
    EXPECT_GT(yt.calculate_match_score(partial_result), 0.4);
    EXPECT_LT(yt.calculate_match_score(partial_result), 0.8);

    // Poor match
    auto poor_result = create_test_result("Completely Different", "Random Artist");
    EXPECT_LT(yt.calculate_match_score(poor_result), 0.3);
}

TEST_F(YoutubeTest, CalculateTitleSimilarityTest)
{
    Youtube yt(create_test_track(), api_key);

    EXPECT_NEAR(yt.calculate_title_similarity("test", "test"), 1.0, 0.01);
    EXPECT_NEAR(yt.calculate_title_similarity("test", "test1"), 0.8, 0.1);
    EXPECT_NEAR(yt.calculate_title_similarity("completely different", "not same"), 0.0, 0.2);
    EXPECT_NEAR(yt.calculate_title_similarity("", ""), 0.0, 0.01);
}

TEST_F(YoutubeTest, NormalizeStringTest)
{
    Youtube yt(create_test_track(), api_key);

    EXPECT_EQ(yt.normalize_string("Test String!"), "test string");
    EXPECT_EQ(yt.normalize_string("  Spaces  "), "spaces");
    EXPECT_EQ(yt.normalize_string("Special@#$Characters"), "special characters");
    EXPECT_EQ(yt.normalize_string(""), "");
}

TEST_F(YoutubeTest, ParseResponseTest)
{
    Youtube yt(create_test_track(), api_key);

    std::string test_response = R"({
        "items": [
            {
                "id": {"videoId": "test123"},
                "snippet": {
                    "title": "Test Track",
                    "channelTitle": "Test Artist"
                }
            }
        ]
    })";

    auto results = yt.parse_response(test_response);
    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].video_id, "test123");
    EXPECT_EQ(results[0].title, "Test Track");
    EXPECT_EQ(results[0].artist, "Test Artist");
}