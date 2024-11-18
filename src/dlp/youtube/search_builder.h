#pragma once
#ifndef SEARCH_BUILDER_H
#define SEARCH_BUILDER_H

#include <string>
#include <regex>
#include <sstream>
#include "../spotify/metadata.h"

using URL = std::string;

class YoutubeSearchBuilder
{
protected:
    URL create_track_search_url(const TrackMetadata &track);
    URL create_album_search_url(const AlbumMetadata &album);

private:
    std::string sanitize_query(const std::string &input);

    URL url_encode(const std::string &input);
    std::string join_artists(const std::vector<Artist> &artists);
};

#endif