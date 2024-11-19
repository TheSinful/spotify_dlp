#include <gtest/gtest.h>
#include <typeinfo>
#include <variant>
#include <regex>
#include "../src/dlp/spotify/metadata.h"
#include "../src/dlp/spotify/api.h"
#include "../src/dlp/youtube/search_builder.h"

class YoutubeSearchBuilderTest : public ::testing::Test
{
protected:
    SpotifyAPI api;
    YoutubeSearchBuilder url_builder;

    YoutubeSearchBuilderTest()
        : api("c620ce69c5ec439f9c519c30dfb7b18c", "b1dc312867a34e9ea764173664b33499"),
          url_builder()
    {
    }

    inline bool is_valid_url(const std::string &url)
    {
        // Basic YouTube URL pattern check
        static const std::regex youtube_pattern(
            R"(https?:\/\/(?:www\.)?youtube\.com\/results\?search_query=[\w\-\+%]+)");

        if (!std::regex_match(url, youtube_pattern))
        {
            return false;
        }

        // Check for minimum length
        if (url.length() < 30)
        {
            return false;
        }

        if (url.find("search_query=") == std::string::npos ||
            url.find("search_query=") == url.length() - 13)
        {
            return false;
        }

        static const std::string invalid_chars = "<>\"'\\";
        if (url.find_first_of(invalid_chars) != std::string::npos)
        {
            return false;
        }

        return true;
    }

    template <typename MetadataType>
    bool create_url(const AnyMetadata &data)
    {
        if (!std::holds_alternative<MetadataType>(data))
        {
            return false;
        }

        URL url;

        if constexpr (std::is_same_v<MetadataType, TrackMetadata>)
        {
            url = this->url_builder.create_track_search_url(std::get<MetadataType>(data));
        }
        else if constexpr (std::is_same_v<MetadataType, AlbumMetadata>)
        {
            url = this->url_builder.create_album_search_url(std::get<MetadataType>(data));
        }
        else if constexpr (std::is_same_v<MetadataType, PlaylistMetadata>)
        {
            // url = url_builder.create_playlist_search_url(std::get<MetadataType>(data));
        }

        return is_valid_url(url);
    }
};

TEST_F(YoutubeSearchBuilderTest, CreateTrackSearchURLTest)
{
    AnyMetadata data = api.get_metadata("https://open.spotify.com/track/0O45fw2L5vsWpdsOdXwNAR?si=f22bdbe688704f34");
    ASSERT_TRUE(create_url<TrackMetadata>(data));
}

TEST_F(YoutubeSearchBuilderTest, CreateAlbumSearchURLTest)
{
    AnyMetadata data = api.get_metadata("https://open.spotify.com/album/2scB1uhcCI1TSf6b9TCZK3?si=egphqqb3QVW23CsJy9GKDQ");
    ASSERT_TRUE(create_url<AlbumMetadata>(data));
}

TEST_F(YoutubeSearchBuilderTest, TestUrlEncodeValid)
{
    std::string unencoded = "test?&=/ special chars!@#$%^&*()";
    std::string encoded = url_builder.url_encode(unencoded);

    // Check if the encoded string contains only allowed characters (regex brought to you by you guess it chatgpt!)
    static const std::regex valid_chars(R"(^[A-Za-z0-9%\-\._~]+$)");
    ASSERT_TRUE(std::regex_match(encoded, valid_chars));
    ASSERT_NE(encoded.find("%"), std::string::npos); // Should contain at least one encoded character
    ASSERT_EQ(encoded.find(" "), std::string::npos); // Should not contain spaces
    ASSERT_EQ(encoded.find("!"), std::string::npos); // Should not contain special characters
}

TEST_F(YoutubeSearchBuilderTest, TestJoinArtistsValid)
{
    Artist artist1{
        .external_urls = {"https://open.spotify.com/artist/1"},
        .id = "1",
        .name = "Artist1",
        .uri = "spotify:artist:1"
    };
    
    Artist artist2{
        .external_urls = {"https://open.spotify.com/artist/2"},
        .id = "2",
        .name = "Artist2",
        .uri = "spotify:artist:2"
    };
    
    Artist artist3{
        .external_urls = {"https://open.spotify.com/artist/3"},
        .id = "3",
        .name = "Artist3",
        .uri = "spotify:artist:3"
    };

    std::vector<Artist> empty_artists;
    ASSERT_EQ(url_builder.join_artists(empty_artists), "");

    std::vector<Artist> single_artist{artist1};
    ASSERT_EQ(url_builder.join_artists(single_artist), "Artist1");

    std::vector<Artist> multiple_artists{artist1, artist2, artist3};
    ASSERT_EQ(url_builder.join_artists(multiple_artists), "Artist1 feat. Artist2, Artist3");
}
