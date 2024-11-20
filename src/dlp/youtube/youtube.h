#pragma once
#ifndef YOUTUBE_H
#define YOUTUBE_H

#include <variant>
#include <nlohmann/json.hpp>
#include "search_builder.h"
#include "../spotify/metadata.h"

using AnyMetadata = std::variant<TrackMetadata, AlbumMetadata, PlaylistMetadata>;

struct SearchResult
{
    std::string video_id;
    std::string title;
    std::string artist;
    int duration_seconds;
    double score;
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
    Youtube(AnyMetadata metadata, std::string yt_api_key);
    std::vector<SearchResult> search();

private:
    bool is_track();
    bool is_album();    // for albums we will try to find an album that exactly matches and download each song from that, if we cannot find an exact match we will just search for all the tracks individually instead.
    bool is_playlist(); // for playlists we will just iterate thru each song and download each like that.

    std::string make_search_request(const std::string &query);
    std::vector<SearchResult> parse_response(const std::string &response);
    std::string url_encode(const std::string &decoded);
    double calculate_match_score(const SearchResult &result);
    double calculate_title_similarity(const std::string &a, const std::string &b);
    std::string normalize_string(const std::string &input);

    // taken from: https://github.com/guilhermeagostinelli/levenshtein/blob/master/levenshtein.cpp
    int levenshtein_distance(std::string a, std::string b);

    YoutubeMusicSearchQueryBuilder url_builder;
    AnyMetadata metadata;
    std::vector<Query> queries;
    std::string api_key;
};

#endif