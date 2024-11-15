#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>

#include "./api.h"
#include "../../utils/curl_utils.h"
#include "../../utils/logger.h"

using namespace std;
using namespace nlohmann;

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

SpotifyAPI::SpotifyAPI(string client_id, string client_secret)
    : client_id(move(client_id)), client_secret(move(client_secret))
{
    fetch_oauth_token();
}

void SpotifyAPI::fetch_oauth_token()
{
    LOG_DEBUG("Fetching token for client ID: ", this->client_id);

    string response;
    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    string data = "grant_type=client_credentials&client_id=" + this->client_id +
                  "&client_secret=" + this->client_secret;

    CURL *curl = this->curl_guard.get();
    if (!curl)
    {
        string log = "Failed to initialize CURL";
        THROW_AND_LOG(CurlException, log, log);
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
        string base_msg = "Failed to send request to fetch authorization token for spotify.";
        string verbose_msg = base_msg + curl_easy_strerror(res) + " (" + to_string(res) + ")";
        THROW_AND_LOG(CurlException, base_msg, verbose_msg);
    }

    string obtain_msg = "Successfully obtained spotify access token";
    LOG_INFO(obtain_msg, obtain_msg);

    try
    {
        json json_response = json::parse(response);
        if (json_response.contains("error"))
        {
            string error = json_response["error"].get<string>();
            string base_log = "Spotify API Error.";
            THROW_AND_LOG(CurlException, base_log, base_log + error);
        }
        this->token = json_response["access_token"].get<string>();
    }
    catch (const json::exception &e)
    {
        string msg = "Failed to parse response when fetching Spotify oauth token.";
        string verbose_msg = msg + string(e.what());
        THROW_AND_LOG(CurlException, msg, verbose_msg);
    }
}

void SpotifyAPI::get_metadata(string url)
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
    json response = this->fetch_spotify_item_json("/tracks");
    return TrackMetadata::serialize(response);
}

AlbumMetadata SpotifyAPI::fetch_album_metadata()
{
    json response = this->fetch_spotify_item_json("/albums");

    return AlbumMetadata::serialize(response);
}

PlaylistMetadata SpotifyAPI::fetch_playlist_metadata()
{
    json response = this->fetch_spotify_item_json("/playlists");

    return PlaylistMetadata::serialize(response);
}

json SpotifyAPI::fetch_spotify_item_json(string endpoint)
{
    string response;
    string url = "https://api.spotify.com/v1" + endpoint + "/" + this->item_id;

    string auth_header = "Authorization: Bearer " + this->token;
    curl_slist *headers = curl_slist_append(nullptr, auth_header.c_str());
    this->curl_guard.set_headers(headers);

    CURL *curl = this->curl_guard.get();
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        string base_log = "Failed to make request to request metadata from Spotify API!";
        THROW_AND_LOG(runtime_error, base_log, base_log + curl_easy_strerror(res));
    }

    json json_response = json::parse(response);
    if (json_response.contains("error"))
    {
        string base_log = "Spotify API returned an error when attempting to request metadata!";
        string response_err = json_response["error"].get<string>();
        THROW_AND_LOG(runtime_error, base_log, base_log + response_err);
    }

    return json_response;
}

void SpotifyAPI::parse_url()
{
    string cleaned_url = validate_and_clean_url(this->spotify_url);
    auto [type, id] = extract_type_and_id(cleaned_url);

    this->download_type = determine_content_type(type);
    this->item_id = id;
}

string SpotifyAPI::validate_and_clean_url(const string &url)
{
    if (url.starts_with("spotify:"))
    {
        throw runtime_error("Spotify URI was passed instead of a playlist, track, or album URL.");
    }

    string cleaned_url = url;
    const string prefix = "https://open.spotify.com/";
    if (cleaned_url.starts_with(prefix))
    {
        cleaned_url = cleaned_url.substr(prefix.length());
    }

    size_t query_pos = cleaned_url.find('?');
    if (query_pos != string::npos)
    {
        cleaned_url = cleaned_url.substr(0, query_pos);
    }

    return cleaned_url;
}

pair<string, string> SpotifyAPI::extract_type_and_id(const string &url)
{
    size_t slash_pos = url.find('/');
    if (slash_pos == string::npos)
    {
        string log = "Invalid URL format";
        THROW_AND_LOG(runtime_error, log, log + "given when attempting to extract type and id off url" + url);
    }

    string type = url.substr(0, slash_pos);
    string id = url.substr(slash_pos + 1);

    if (type.starts_with("intl-"))
    {
        size_t second_slash = id.find('/');
        if (second_slash != string::npos)
        {
            type = id.substr(0, second_slash);
            id = id.substr(second_slash + 1);
        }
    }

    return {type, id};
}

DownloadType SpotifyAPI::determine_content_type(const string &type)
{
    if (type == "album")
        return DownloadType::Album;
    if (type == "playlist")
        return DownloadType::Playlist;
    if (type == "track")
        return DownloadType::Track;
    throw runtime_error("Unknown content type: " + type);
}
