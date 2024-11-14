#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include "api.h"
#include "../utils/curl_utils.h"
#include "../utils/logger.h"

namespace
{
    struct GlobalCurlInit
    {
        GlobalCurlInit()
        {
            if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK)
            {
                throw CurlException("Failed to initialize CURL globally");
            }
        }
        ~GlobalCurlInit()
        {
            curl_global_cleanup();
        }
    };

    static GlobalCurlInit globalInit;
}

SpotifyAPI::SpotifyAPI(std::string client_id, std::string client_secret)
    : client_id(std::move(client_id)), client_secret(std::move(client_secret))
{
    fetch_token();
}

void SpotifyAPI::fetch_token()
{
    LOG_DEBUG("Fetching token for client ID: ", this->client_id);

    std::string response;
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    std::string data = "grant_type=client_credentials&client_id=" + this->client_id +
                       "&client_secret=" + this->client_secret;

    CURL *curl = this->curl_guard.get();
    if (!curl)
    {
        std::string log = "Failed to initialize CURL";
        THROW_WITH_LOG(CurlException, log, log);
    }

    this->curl_guard.set_headers(headers);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::string base_msg = "Failed to send request to fetch authorization token for spotify.";
        std::string verbose_msg = base_msg + curl_easy_strerror(res) + " (" + std::to_string(res) + ")";
        THROW_WITH_LOG(CurlException, base_msg, verbose_msg);
    }

    std::string obtain_msg = "Successfully obtained spotify access token";
    LOG_INFO(obtain_msg, obtain_msg);

    try
    {
        nlohmann::json json_response = nlohmann::json::parse(response);
        if (json_response.contains("error"))
        {
            std::string error = json_response["error"].get<std::string>();
            std::string base_log = "Spotify API Error.";
            THROW_WITH_LOG(CurlException, base_log, base_log + error);
        }
        this->token = json_response["access_token"].get<std::string>();
    }
    catch (const nlohmann::json::exception &e)
    {

        std::string msg = "Failed to parse response from spotify auth token request.";
        THROW_WITH_LOG(CurlException, msg, msg + std::string(e.what()));
    }
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
        this->metadata = this->fetch_playlist_metadata();
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

    return AlbumMetadata::serialize(response);
}

PlaylistMetadata SpotifyAPI::fetch_playlist_metadata()
{
    nlohmann::json response = this->fetch_raw_metadata("/playlists");

    return PlaylistMetadata::serialize(response);
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
