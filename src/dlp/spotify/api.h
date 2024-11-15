#pragma once
#ifndef API_H
#define API_H

#include <string>
#include <variant>
#include <nlohmann/json.hpp>
#include <gtest/gtest.h>
#include "../../utils/curl_utils.h"
#include "metadata.h"

class SpotifyAPITest;

class SpotifyAPI
{
public:
    SpotifyAPI(std::string client_id, std::string client_secret);
    void get_metadata(std::string url);

private:
    std::string validate_and_clean_url(const std::string &url);                      // tested
    std::pair<std::string, std::string> extract_type_and_id(const std::string &url); // tested
    DownloadType determine_content_type(const std::string &type);                    // tested

    void parse_url();                                             // tested
    void fetch_oauth_token();                                     // tested
    TrackMetadata fetch_track_metadata();                         // tested
    AlbumMetadata fetch_album_metadata();                         // tested
    PlaylistMetadata fetch_playlist_metadata();                   // tested
    nlohmann::json fetch_spotify_item_json(std::string endpoint); // tested

    std::string spotify_url;
    std::string token;
    std::string client_id;
    std::string client_secret;
    std::string item_id;
    std::variant<TrackMetadata, AlbumMetadata, PlaylistMetadata> metadata;
    DownloadType download_type;

    CurlGuard curl_guard;

    // testing
    friend class SpotifyAPITest;

    FRIEND_TEST(SpotifyAPITest, ValidateAndCleanUrlRemovesQueryParameters);
    FRIEND_TEST(SpotifyAPITest, ValidateAndCleanUrlHandlesShortUrl);
    FRIEND_TEST(SpotifyAPITest, ValidateAndCleanUrlThrowsOnSpotifyUri);
    FRIEND_TEST(SpotifyAPITest, SplitUrlParsesCorrectly);
    FRIEND_TEST(SpotifyAPITest, DetermineContentTypeReturnsCorrectType);
    FRIEND_TEST(SpotifyAPITest, ValidateFetchToken);
    FRIEND_TEST(SpotifyAPITest, FetchTrackMetadataReturnsCorrectData);
    FRIEND_TEST(SpotifyAPITest, FetchAlbumMetadataReturnsCorrectData);
    FRIEND_TEST(SpotifyAPITest, FetchPlaylistMetadataReturnsCorrectData);
};

#endif