#include <optional>
#include <string>
#include <vector>

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

    static Artist serialize(const nlohmann::json &data);
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

    static TrackMetadata serialize(const nlohmann::json &data);
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

TrackMetadata TrackMetadata::serialize(const nlohmann::json &data)
{
    TrackMetadata track;
    track.name = data["name"];

    std::vector<Artist> artists;
    for (const auto &artist_data : data["artists"])
    {
        artists.push_back(Artist::serialize(artist_data));
    }

    track.artists = artists;
    track.spotify_id = data["id"];

    return track;
}

Artist Artist::serialize(const nlohmann::json &data)
{
    Artist artist;
    for (const auto &[service, url] : data["external_urls"].items())
    {
        artist.external_urls.push_back(url.get<std::string>());
    }

    artist.id = data["id"];
    artist.name = data["name"];
    artist.uri = data["uri"];

    return artist;
}