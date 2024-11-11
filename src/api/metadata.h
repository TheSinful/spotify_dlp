#include <optional> 
#include <string> 
#include <vector> 

enum DownloadType
{
    Album,
    Playlist,
    Track
};

struct BaseMetadata {
    std::string name;
    std::string artist;
    std::string spotify_id;
    std::optional<std::string> cover;
    std::optional<std::string> genre;
    std::optional<float> tempo;
};

struct TrackMetadata : BaseMetadata {
    std::optional<std::string> album;
    std::optional<std::string> year;
    int track_number;
};

struct AlbumMetadata : BaseMetadata {
    std::string year;
    int total_tracks;
    std::vector<TrackMetadata> tracks;
};

struct PlaylistMetadata : BaseMetadata {
    std::vector<TrackMetadata> tracks;
    int total_tracks;
};