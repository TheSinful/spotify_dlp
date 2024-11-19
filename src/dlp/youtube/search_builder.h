#pragma once
#ifndef SEARCH_BUILDER_H
#define SEARCH_BUILDER_H

#include <string>
#include <regex>
#include <sstream>
#include "../spotify/metadata.h"
#ifdef BUILD_TEST
#include <gtest/gtest.h>
#endif

using URL = std::string;

class YoutubeSearchBuilder
{
#ifdef BUILD_TEST
    friend class YoutubeSearchBuilderTest; 

    FRIEND_TEST(YoutubeSearchBuilderTest, CreateTrackSearchURLTest);
    FRIEND_TEST(YoutubeSearchBuilderTest, CreateAlbumSearchURLTest);
    FRIEND_TEST(YoutubeSearchBuilderTest, TestUrlEncodeValid); 
    FRIEND_TEST(YoutubeSearchBuilderTest, TestJoinArtistsValid);
#endif

protected:
    URL create_track_search_url(const TrackMetadata &track);
    URL create_album_search_url(const AlbumMetadata &album);
private:
    URL url_encode(const std::string &input);
    std::string sanitize_query(const std::string &input);
    std::string join_artists(const std::vector<Artist> &artists);
};

#endif