#include "metadata.h"

Artist Artist::serialize(const nlohmann::json &data)
{
    std::vector<std::string> urls;
    for (const auto &[service, url] : data["external_urls"].items())
    {
        urls.push_back(url.get<std::string>());
    }

    Artist artist;
    artist.external_urls = urls;
    artist.id = data["id"];
    artist.name = data["name"];
    artist.uri = data["uri"];

    return artist;
}

TrackMetadata TrackMetadata::serialize(const nlohmann::json &data)
{
    TrackMetadata track;
    track.name = data["name"];
    track.track_number = data["track_number"];
    track.id = data["id"];
    track.album_name = data["album"]["name"];
    track.album_id = data["album"]["id"];
    
    // Set artists
    std::vector<Artist> artists;
    for (const auto &artist : data["artists"])
    {
        artists.push_back(Artist::serialize(artist));
    }
    track.artists = artists;

    // Set external URLs
    std::vector<std::string> external_urls;
    for (const auto &[name, value] : data["external_urls"].items())
    {
        external_urls.push_back(value);
    }
    track.external_urls = external_urls;

    return track;
}

AlbumMetadata AlbumMetadata::serialize(const nlohmann::json &data)
{
    AlbumMetadata album;
    album.name = data["name"];
    std::vector<Artist> artists;
    for (const auto &artist : data["artists"])
    {
        artists.push_back(Artist::serialize(artist));
    };
    album.id = data["id"];
    album.release_date = data["release_date"];
    album.total_tracks = data["tracks"]["total"];
    std::vector<TrackMetadata> tracks;
    for (const auto &track : data["tracks"]["items"])
    {
        tracks.push_back(TrackMetadata::serialize(track));
    }

    return album;
}

PlaylistMetadata PlaylistMetadata::serialize(const nlohmann::json &data)
{
    PlaylistMetadata playlist;
    playlist.name = data["name"];
    playlist.id = data["id"];
    playlist.total_tracks = data["tracks"]["total"];
    std::vector<TrackMetadata> tracks;
    for (const auto &track : data["tracks"]["items"])
    {
        tracks.push_back(TrackMetadata::serialize(track));
    }
    playlist.tracks = tracks;

    return playlist;
}