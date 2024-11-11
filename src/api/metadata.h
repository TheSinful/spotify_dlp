#ifndef METADATA_H
#define METADATA_H

#include <optional>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum DownloadType
{
    Album,
    Playlist,
    Track
};

struct Artist
{
    std::vector<std::string> external_urls;
    std::string id;
    std::string name;
    std::string uri;

    static Artist serialize(const nlohmann::json &data);  // declaration only
};

struct BaseMetadata
{
    std::string name;
    std::optional<std::vector<Artist>> artists; // not set in a playlist
    std::string spotify_id;
    std::optional<std::string> cover;
    std::optional<std::vector<std::string>> genres;
    std::optional<float> tempo;
};

struct TrackMetadata : BaseMetadata
{
    std::optional<std::string> album;
    std::optional<std::string> year;
    int track_number;

    static TrackMetadata serialize(const nlohmann::json &data);  // declaration only
};

struct AlbumMetadata : BaseMetadata
{
    std::string release_date;
    int total_tracks;
    std::vector<TrackMetadata> tracks;
};

struct PlaylistMetadata : BaseMetadata
{
    std::vector<TrackMetadata> tracks;
    int total_tracks;
};

#endif