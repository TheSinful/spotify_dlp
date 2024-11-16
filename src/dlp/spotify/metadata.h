#pragma once
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
    bool operator==(const Artist &other) const
    {
        return external_urls == other.external_urls &&
               id == other.id &&
               name == other.name &&
               uri == other.uri;
    }

    std::vector<std::string> external_urls;
    std::string id;
    std::string name;
    std::string uri;

    static Artist serialize(const nlohmann::json &data); // declaration only
};

struct TrackMetadata
{
    bool operator==(const TrackMetadata &other) const
    {
        if (this == &other)
            return true;

        return id == other.id &&
               name == other.name &&
               track_number == other.track_number &&
               artists == other.artists &&
               album_id == other.album_id &&
               album_name == other.album_name &&
               external_urls == other.external_urls;
    }

    static TrackMetadata serialize(const nlohmann::json &data);

    std::string name;
    std::vector<Artist> artists;
    std::string id;
    std::optional<std::string> album_name;
    std::optional<std::string> album_id;
    std::vector<std::string> external_urls;
    int track_number;

    //TODO: Add duration_ms and available market for better search engine results from youtube.
};

struct AlbumMetadata
{
    bool operator==(const AlbumMetadata &other) const
    {
        if (this == &other)
            return true;

        return id == other.id &&
               name == other.name &&
               release_date == other.release_date &&
               total_tracks == other.total_tracks &&
               artists == other.artists &&
               tracks == other.tracks;
    }

    static AlbumMetadata serialize(const nlohmann::json &data);

    std::string name;
    std::vector<Artist> artists;
    std::string id;
    std::string release_date;
    int total_tracks;
    std::vector<TrackMetadata> tracks;
};

struct PlaylistMetadata
{
    std::string name;
    std::string id;
    std::vector<TrackMetadata> tracks;
    int total_tracks;

    static PlaylistMetadata serialize(const nlohmann::json &data);

    bool operator==(const PlaylistMetadata &other) const
    {
        if (this == &other)
            return true;

        return total_tracks == other.total_tracks &&
               id == other.id &&
               name == other.name &&
               tracks == other.tracks;
    }; 
};

#endif