#include <gtest/gtest.h>
#include <typeinfo>
#include <variant>
#include <regex>
#include "../src/dlp/spotify/metadata.h"
#include "../src/dlp/spotify/api.h"
#include "../src/dlp/youtube/search_builder.h"

class YoutubeSearchQueryBuilderTest : public ::testing::Test
{
protected:
    SpotifyAPI api;
    YoutubeMusicSearchQueryBuilder query_builder;

    YoutubeSearchQueryBuilderTest()
        : api("c620ce69c5ec439f9c519c30dfb7b18c", "b1dc312867a34e9ea764173664b33499"),
          query_builder()
    {
    }

    inline bool is_valid_query(const std::string &query)
    {
        if (query.empty())
        {
            return false;
        }

        // Query should not contain URL-unsafe characters
        static const std::string invalid_chars = "<>\"'\\";
        if (query.find_first_of(invalid_chars) != std::string::npos)
        {
            return false;
        }

        return true;
    }

    template <typename MetadataType>
    bool create_query(const AnyMetadata &data)
    {
        if (!std::holds_alternative<MetadataType>(data))
        {
            return false;
        }

        std::string query;

        if constexpr (std::is_same_v<MetadataType, TrackMetadata>)
        {
            query = this->query_builder.create_track_search_query(std::get<MetadataType>(data));
        }
        else if constexpr (std::is_same_v<MetadataType, AlbumMetadata>)
        {
            query = this->query_builder.create_album_search_query(std::get<MetadataType>(data));
        }

        return is_valid_query(query);
    }
};

TEST_F(YoutubeSearchQueryBuilderTest, CreateTrackSearchQueryTest)
{
    AnyMetadata data = api.get_metadata("https://open.spotify.com/track/0O45fw2L5vsWpdsOdXwNAR?si=f22bdbe688704f34");
    ASSERT_TRUE(create_query<TrackMetadata>(data));
}

TEST_F(YoutubeSearchQueryBuilderTest, CreateAlbumSearchQueryTest)
{
    AnyMetadata data = api.get_metadata("https://open.spotify.com/album/2scB1uhcCI1TSf6b9TCZK3?si=egphqqb3QVW23CsJy9GKDQ");
    ASSERT_TRUE(create_query<AlbumMetadata>(data));
}

TEST_F(YoutubeSearchQueryBuilderTest, TestJoinArtistsValid)
{
    Artist artist1{
        .external_urls = {"https://open.spotify.com/artist/1"},
        .id = "1",
        .name = "Artist1",
        .uri = "spotify:artist:1"};

    Artist artist2{
        .external_urls = {"https://open.spotify.com/artist/2"},
        .id = "2",
        .name = "Artist2",
        .uri = "spotify:artist:2"};

    Artist artist3{
        .external_urls = {"https://open.spotify.com/artist/3"},
        .id = "3",
        .name = "Artist3",
        .uri = "spotify:artist:3"};

    std::vector<Artist> empty_artists;
    ASSERT_EQ(query_builder.join_artists(empty_artists), "");

    std::vector<Artist> single_artist{artist1};
    ASSERT_EQ(query_builder.join_artists(single_artist), "Artist1");

    std::vector<Artist> multiple_artists{artist1, artist2, artist3};
    ASSERT_EQ(query_builder.join_artists(multiple_artists), "Artist1 feat. Artist2, Artist3");
}
