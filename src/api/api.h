#include <string>
#include <variant>
#include "metadata.h"

class SpotifyAPI
{
public:
    SpotifyAPI(std::string client_id, std::string client_secret);
    void get_metadata(std::string url);

private:
    void parse_url();
    void fetch_token();
    TrackMetadata fetch_track_metadata();
    AlbumMetadata fetch_album_metadata();
    PlaylistMetadata fetch_playlist_metadata();
    nlohmann::json fetch_raw_metadata(std::string endpoint);

    std::string validate_and_clean_url(const std::string &url);
    std::pair<std::string, std::string> split_url(const std::string &url);
    DownloadType determine_content_type(const std::string &type);

    std::string spotify_url;
    std::string token;
    std::string client_id;
    std::string client_secret;
    std::string item_id;
    std::variant<TrackMetadata, AlbumMetadata, PlaylistMetadata> metadata;
    DownloadType download_type;

    CurlGuard curl_guard;
};
