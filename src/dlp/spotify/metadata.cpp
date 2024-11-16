#include "metadata.h"
#include "../../utils/logger.h"

Artist Artist::serialize(const nlohmann::json &data)
{
    if (!data.contains("id") || data["id"].is_null() ||
        !data.contains("name") || data["name"].is_null() ||
        !data.contains("uri") || data["uri"].is_null() ||
        !data.contains("external_urls") || data["external_urls"].is_null())
    {
        std::string log = "Missing required fields in artist JSON response";
        THROW_AND_LOG(std::runtime_error, log, log);
    }

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
    if (!data.contains("name") || data["name"].is_null() ||
        !data.contains("id") || data["id"].is_null() ||
        !data.contains("track_number") || data["track_number"].is_null() ||
        !data.contains("album") || data["album"].is_null() ||
        !data.contains("artists") || data["artists"].is_null())
    {
        std::string log = "Missing required fields in track JSON response";
        THROW_AND_LOG(std::runtime_error, log, log);
    }

    TrackMetadata track;
    track.name = data["name"];
    track.track_number = data["track_number"];
    track.id = data["id"];
    track.album_name = data["album"]["name"];
    track.album_id = data["album"]["id"];

    std::vector<Artist> artists;
    for (const auto &artist : data["artists"])
    {
        artists.push_back(Artist::serialize(artist));
    }
    track.artists = artists;

    if (artists.empty())
    {
        LOG_WARN("Track has no artists", "Track {} (ID: {}) has no artists!", track.name, track.id);
    }

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
    if (!data.contains("name") || data["name"].is_null() ||
        !data.contains("id") || data["id"].is_null() ||
        !data.contains("release_date") || data["release_date"].is_null() ||
        !data.contains("total_tracks") || data["total_tracks"].is_null() ||
        !data.contains("artists") || data["artists"].is_null())
    {
        std::string log = "Missing required fields in album JSON response";
        THROW_AND_LOG(std::runtime_error, log, log);
    }

    AlbumMetadata album;
    album.name = data["name"];
    album.id = data["id"];
    album.release_date = data["release_date"];
    album.total_tracks = data["total_tracks"];

    std::vector<Artist> artists;
    for (const auto &artist : data["artists"])
    {
        artists.push_back(Artist::serialize(artist));
    }
    album.artists = artists;

    if (artists.empty())
    {
        LOG_WARN("Album has no artists", "Album {} (ID: {}) has no artists!", album.name, album.id);
    }

    if (album.total_tracks == 0)
    {
        LOG_WARN("Album has no tracks", "Album {} (ID: {}) has zero tracks!", album.name, album.id);
    }

    std::vector<TrackMetadata> tracks;
    for (const auto &track : data["tracks"]["items"])
    {
        auto track_data = track;
        track_data["album"] = {
            {"name", album.name},
            {"id", album.id}};
        tracks.push_back(TrackMetadata::serialize(track_data));
    }
    album.tracks = tracks;

    return album;
}

PlaylistMetadata PlaylistMetadata::serialize(const nlohmann::json &data)
{
    PlaylistMetadata playlist;

    if (!data.contains("name") || data["name"].is_null() ||
        !data.contains("id") || data["id"].is_null() ||
        !data.contains("tracks") || data["tracks"].is_null())
    {
        std::string log = "Missing required fields in playlist JSON response";
        THROW_AND_LOG(std::runtime_error, log, log);
    }

    playlist.name = data["name"].get<std::string>();
    playlist.id = data["id"].get<std::string>();
    playlist.total_tracks = data["tracks"].value("total", 0);

    if (playlist.total_tracks == 0)
    {
        LOG_WARN("Playlist has no tracks.", "Playlist {} (ID: {}) has zero tracks inside!", playlist.name, playlist.id);
    };

    std::vector<TrackMetadata> tracks;
    if (data["tracks"].contains("items") && data["tracks"]["items"].is_array())
    {
        for (const auto &item : data["tracks"]["items"])
        {
            if (item.contains("track") && !item["track"].is_null())
            {
                tracks.push_back(TrackMetadata::serialize(item["track"]));
            }
        }
    }
    playlist.tracks = tracks;

    return playlist;
}