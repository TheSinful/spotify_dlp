#pragma once
#ifndef YOUTUBE_H
#define YOUTUBE_H

#include <variant>
#include <nlohmann/json.hpp>
#include "../../../include/spotify-dlp.h"
#include "../spotify/metadata.h"
#include "yt-dlp.h"
#include "search_builder.h"

using AnyMetadata = std::variant<TrackMetadata, AlbumMetadata, PlaylistMetadata>;
using URL = std::string;

struct SearchResult
{
    std::string video_id;
    std::string title;
    std::string artist;
    int duration_seconds;
    double score;
};

class RateLimitException : public std::exception
{
public:
    explicit RateLimitException(const std::string &message) : msg(message) {}
    const char *what() const noexcept override { return msg.c_str(); }

private:
    std::string msg;
};

class Youtube
{
#ifdef BUILD_TEST
    friend class YoutubeTest;
    FRIEND_TEST(YoutubeTest, CalculateMatchScoreTrackTest);
    FRIEND_TEST(YoutubeTest, CalculateMatchScoreAlbumTest);
    FRIEND_TEST(YoutubeTest, CalculateTitleSimilarityTest);
    FRIEND_TEST(YoutubeTest, NormalizeStringTest);
    FRIEND_TEST(YoutubeTest, ParseResponseTest);
#endif

public:
    Youtube(std::string yt_api_key, DownloadConfig config);
    std::vector<URL> search(AnyMetadata metadata);
    std::vector<std::filesystem::path> download();

private:
    // searching
    bool is_track();
    bool is_album();    // for albums we will try to find an album that exactly matches and download each song from that, if we cannot find an exact match we will just search for all the tracks individually instead.
    bool is_playlist(); // for playlists we will just iterate thru each song and download each like that.
    std::string make_search_request(Query query);
    std::vector<SearchResult> parse_response(const std::string &response); // since we may also search for an album
    double calculate_match_score(const SearchResult &result);
    double calculate_track_score(const SearchResult &result, const TrackMetadata &track);
    double calculate_album_score(const SearchResult &result, const AlbumMetadata &album);
    double calculate_playlist_score(const SearchResult &result); // Currently returns 0
    std::string url_encode(const std::string &decoded);
    double calculate_title_similarity(const std::string &a, const std::string &b);
    std::string normalize_string(const std::string &input);
    // taken from: https://github.com/guilhermeagostinelli/levenshtein/blob/master/levenshtein.cpp
    int levenshtein_distance(std::string a, std::string b);
    URL get_music_url(SearchResult search_result);

    YoutubeMusicSearchQueryBuilder query_builder;
    AnyMetadata metadata;
    std::vector<Query> queries;
    std::string api_key;

    // downloading
    YtDLP downloader;
    DownloadConfig config;
};

#endif