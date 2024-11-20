#include "search_builder.h"

Query YoutubeMusicSearchQueryBuilder::create_track_search_query(const TrackMetadata &track)
{
    std::string artistString = this->join_artists(track.artists);
    return artistString + " " + track.name + 
           (track.album_name.has_value() ? " " + track.album_name.value() : "");
}

Query YoutubeMusicSearchQueryBuilder::create_album_search_query(const AlbumMetadata &album)
{
    std::string artistString = this->join_artists(album.artists);
    return artistString + " " + album.name + " album";
}

std::string YoutubeMusicSearchQueryBuilder::join_artists(const std::vector<Artist> &artists)
{
    if (artists.empty())
        return "";

    std::stringstream ss;
    ss << artists[0].name; // Primary artist

    if (artists.size() > 1)
    {
        ss << " feat.";
        for (size_t i = 1; i < artists.size(); ++i)
        {
            if (i == artists.size() - 1)
            {
                ss << " " << artists[i].name;
            }
            else
            {
                ss << " " << artists[i].name << ",";
            }
        }
    }
    return ss.str();
}