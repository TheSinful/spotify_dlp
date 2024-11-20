#pragma once
#ifndef SEARCH_BUILDER_H
#define SEARCH_BUILDER_H1

#include <string>
#include <regex>
#include <sstream>
#include "../spotify/metadata.h"
#ifdef BUILD_TEST
#include <gtest/gtest.h>
#endif

using Query = std::string;

class YoutubeMusicSearchQueryBuilder
{
#ifdef BUILD_TEST
    friend class YoutubeSearchQueryBuilderTest;

    FRIEND_TEST(YoutubeSearchQueryBuilderTest, CreateTrackSearchQueryTest);
    FRIEND_TEST(YoutubeSearchQueryBuilderTest, CreateAlbumSearchQueryTest);
    FRIEND_TEST(YoutubeSearchQueryBuilderTest, TestJoinArtistsValid);
#endif

public:
    Query create_track_search_query(const TrackMetadata &track);
    Query create_album_search_query(const AlbumMetadata &album);
    std::string join_artists(const std::vector<Artist> &artists);
};

#endif