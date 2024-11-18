#pragma once
#ifndef YOUTUBE_H
#define YOUTUBE_H

#include <variant>
#include "../spotify/metadata.h";

using AnyMetadata = std::variant<TrackMetadata, AlbumMetadata, PlaylistMetadata>;

// TODO: url search creator to find a song using spotify metadata on youtube
class Youtube
{
public:
    Youtube(AnyMetadata metadata);

private:
    bool is_track();
    bool is_album();    // for albums we will try to find an album that exactly matches and download each song from that, if we cannot find an exact match we will just search for all the tracks individually instead.
    bool is_playlist(); // for playlists we will just iterate thru each song and download each like that.

    


    AnyMetadata metadata;
};

#endif