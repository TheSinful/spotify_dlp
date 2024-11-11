#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>

#include "api.h"
#include "../utils/curl_utils.h"

SpotifyAPI::SpotifyAPI(std::string client_id, std::string client_secret)
{
    CurlGuard guard;
    this->curl_guard = guard;
    this->client_id = client_id;
    this->client_secret = client_secret;
    this->fetch_token();
}

void SpotifyAPI::fetch_token()
{
    std::string response;
    std::string auth_credentials = "grant_type=client_credentials&client_id=" +
                                   client_id + "&client_secret=" + client_secret;

    curl_slist *headers = curl_slist_append(nullptr, "Content-Type: application/x-www-form-urlencoded");
    this->curl_guard.set_headers(headers);
    CURL *curl = this->curl_guard.get();

    curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, auth_credentials.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        throw std::runtime_error(curl_easy_strerror(res));
    }

    nlohmann::json json_response = nlohmann::json::parse(response);
    if (json_response.contains("error"))
    {
        throw std::runtime_error(json_response["error"].get<std::string>());
    }

    this->token = json_response["access_token"].get<std::string>();
}

void SpotifyAPI::get_metadata(std::string url)
{
    this->spotify_url = url;
    this->parse_url();

    switch (download_type)
    {
    case DownloadType::Track:
        this->metadata = this->fetch_track_metadata();
        break;
    case DownloadType::Album:
        this->metadata = this->fetch_album_metadata();
        break;
    case DownloadType::Playlist:
        this->metadata =->fetch_playlist_metadata();
        break;
    }
}

TrackMetadata SpotifyAPI::fetch_track_metadata()
{
    nlohmann::json response = this->fetch_raw_metadata("/tracks");
    return TrackMetadata::serialize(response);
}

AlbumMetadata SpotifyAPI::fetch_album_metadata()
{
    nlohmann::json response = this->fetch_raw_metadata("/albums");

    AlbumMetadata data;
    data.spotify_id = response["id"];
    data.total_tracks = response["total_tracks"];
    data.genres = response["genres"];
    data.name = response["name"];
    data.release_date = response["release_date"];
    data.total_tracks = response["tracks"]["total"];
    std::vector<Artist> artists;
    for (const auto &artist_data : response["artists"])
    {
        artists.push_back(Artist::serialize(artist_data));
    }
    data.artists = artists;

    return data;
}

PlaylistMetadata SpotifyAPI::fetch_playlist_metadata()
{
    nlohmann::json response = this->fetch_raw_metadata("/playlists");

    PlaylistMetadata data;

    data.total_tracks = response["tracks"]["total"];

    std::vector<TrackMetadata> tracks;
    for (const auto &item : response["tracks"]["items"])
    {
        tracks.push_back(TrackMetadata::serialize(item));
    }

    data.tracks = tracks;
    data.spotify_id = response["id"];
    data.cover = response["images"]["url"];

    return data;
}

nlohmann::json SpotifyAPI::fetch_raw_metadata(std::string endpoint)
{
    std::string response;
    std::string url = "https://api.spotify.com/v1" + endpoint + this->item_id;

    std::string auth_header = "Authorization: Bearer " + this->token;
    curl_slist *headers = curl_slist_append(nullptr, auth_header.c_str());
    this->curl_guard.set_headers(headers);

    CURL *curl = this->curl_guard.get();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        throw std::runtime_error(curl_easy_strerror(res));
    }

    nlohmann::json json_response = nlohmann::json::parse(response);
    if (json_response.contains("error"))
    {
        throw std::runtime_error(json_response["error"].get<std::string>());
    }

    return json_response;
}

void SpotifyAPI::parse_url()
{
    std::string cleaned_url = validate_and_clean_url(this->spotify_url);
    auto [type, id] = split_url(cleaned_url);

    this->download_type = determine_content_type(type);
    this->item_id = id;
}

std::string SpotifyAPI::validate_and_clean_url(const std::string &url)
{
    if (url.starts_with("spotify:"))
    {
        throw std::runtime_error("Spotify URI was passed instead of a playlist, track, or album URL.");
    }

    std::string cleaned_url = url;
    const std::string prefix = "https://open.spotify.com/";
    if (cleaned_url.starts_with(prefix))
    {
        cleaned_url = cleaned_url.substr(prefix.length());
    }

    size_t query_pos = cleaned_url.find('?');
    if (query_pos != std::string::npos)
    {
        cleaned_url = cleaned_url.substr(0, query_pos);
    }

    return cleaned_url;
}

std::pair<std::string, std::string> SpotifyAPI::split_url(const std::string &url)
{
    size_t slash_pos = url.find('/');
    if (slash_pos == std::string::npos)
    {
        throw std::runtime_error("Invalid URL format");
    }

    std::string type = url.substr(0, slash_pos);
    std::string id = url.substr(slash_pos + 1);

    if (type.starts_with("intl-"))
    {
        size_t second_slash = id.find('/');
        if (second_slash != std::string::npos)
        {
            type = id.substr(0, second_slash);
            id = id.substr(second_slash + 1);
        }
    }

    return {type, id};
}

DownloadType SpotifyAPI::determine_content_type(const std::string &type)
{
    if (type == "album")
        return DownloadType::Album;
    if (type == "playlist")
        return DownloadType::Playlist;
    if (type == "track")
        return DownloadType::Track;
    throw std::runtime_error("Unknown content type: " + type);
}
