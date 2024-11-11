
#include "metadata.h"

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