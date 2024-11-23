#include <stdexcept>
#include "youtube.h"
#include "../../utils/curl_utils.h"
#include "../../utils/logger.h"

using namespace std;

Youtube::Youtube(string yt_api_key, DownloadConfig config)
    : query_builder(), queries(), api_key(yt_api_key), config(config), downloader() {};

bool Youtube::is_track()
{
    return holds_alternative<TrackMetadata>(this->metadata);
}

bool Youtube::is_album()
{
    return holds_alternative<AlbumMetadata>(this->metadata);
}

bool Youtube::is_playlist()
{
    return holds_alternative<PlaylistMetadata>(this->metadata);
}

string Youtube::make_search_request(Query query)
{
    LOG_INFO("Making search request", "Making search request for query" + query);
    string response;

    CurlGuard curl;

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl.set_headers(headers);
    string header_log = "Created search request headers";
    LOG_INFO(header_log, header_log);

    string url = "https://youtube.googleapis.com/youtube/v3/search"
                 "?part=snippet"
                 "&q=" +
                 url_encode(query) +
                 "&type=video"
                 "&videoCategoryId=10" // Music category
                 "&key=" +
                 api_key;

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response);
    string body_log = "Created search request body, making request!";
    LOG_INFO(body_log, body_log);

    CURLcode res = curl_easy_perform(curl.get());
    if (res != CURLE_OK)
    {
        long http_code = 0;
        curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code == 429)
        {
            THROW_AND_LOG(RateLimitException,
                          "YouTube API rate limit exceeded",
                          "Received 429 Too Many Requests from YouTube API");
        }

        THROW_AND_LOG(CurlException,
                      "CURL request failed: " + string(curl_easy_strerror(res)),
                      "YouTube API request failed with code: " + to_string(res));
    }

    return response;
}

string Youtube::url_encode(const string &decoded)
{
    const auto encoded_value = curl_easy_escape(nullptr, decoded.c_str(), static_cast<int>(decoded.length()));
    string result(encoded_value);
    curl_free(encoded_value);
    return result;
}

vector<SearchResult> Youtube::parse_response(const string &response)
{
    vector<SearchResult> results;
    auto json = nlohmann::json::parse(response);

    if (!json.contains("items"))
    {
        THROW_AND_LOG(runtime_error, "Failed to find items in response", "Failed to find items in response from Youtube API ");
    }

    for (const auto &item : json["items"])
    {
        SearchResult result;
        result.video_id = item["id"]["videoId"];
        result.title = item["snippet"]["title"];
        result.artist = item["snippet"]["channelTitle"];
        result.score = calculate_match_score(result);
        results.push_back(result);
    }

    // Sort results by score in descending order
    sort(results.begin(), results.end(),
         [](const SearchResult &a, const SearchResult &b)
         {
             return a.score > b.score;
         });

    return results;
}

double Youtube::calculate_match_score(const SearchResult &result)
{
    if (this->is_track())
    {
        return calculate_track_score(result, get<TrackMetadata>(metadata));
    }
    else if (this->is_album())
    {
        return calculate_album_score(result, get<AlbumMetadata>(metadata));
    }
    return calculate_playlist_score(result);
}

double Youtube::calculate_track_score(const SearchResult &result, const TrackMetadata &track)
{
    double score = 0.0;

    // Title match (45%)
    double title_score = calculate_title_similarity(
        normalize_string(result.title),
        normalize_string(track.name));
    score += title_score * 0.45;

    // Artist match (45%)
    string track_artists = normalize_string(query_builder.join_artists(track.artists));
    string result_artists = normalize_string(result.artist);
    double artist_score = calculate_title_similarity(track_artists, result_artists);
    score += artist_score * 0.45;

    // Album match bonus (10%)
    if (track.album_name.has_value())
    {
        double album_score = calculate_title_similarity(
            normalize_string(result.title),
            normalize_string(track.album_name.value()));
        score += album_score * 0.10;
    }

    return score;
}

double Youtube::calculate_album_score(const SearchResult &result, const AlbumMetadata &album)
{
    double score = 0.0;

    // Album title match (50%)
    double title_score = calculate_title_similarity(
        normalize_string(result.title),
        normalize_string(album.name));
    score += title_score * 0.50;

    // Artist match (40%)
    string album_artists = normalize_string(query_builder.join_artists(album.artists));
    string result_artists = normalize_string(result.artist);
    double artist_score = calculate_title_similarity(album_artists, result_artists);
    score += artist_score * 0.40;

    // Additional scoring for keywords suggesting it's an album (10%)
    string normalized_title = normalize_string(result.title);
    if (normalized_title.find("album") != string::npos ||
        normalized_title.find("full album") != string::npos)
    {
        score += 0.10;
    }

    return score;
}

double Youtube::calculate_playlist_score(const SearchResult &result)
{
    return 0.0; // Playlists are handled as individual tracks
}

double Youtube::calculate_title_similarity(const string &a, const string &b)
{
    const auto max_len = max(a.length(), b.length());
    if (max_len == 0)
        return 0.0;

    const auto distance = levenshtein_distance(a, b);
    return 1.0 - (static_cast<double>(distance) / max_len);
}

int Youtube::levenshtein_distance(string a, string b)
{
    int size1 = a.size();
    int size2 = b.size();
    int verif[size1 + 1][size2 + 1];

    if (size1 == 0)
        return size2;
    if (size2 == 0)
        return size1;

    for (int i = 0; i <= size1; i++)
        verif[i][0] = i;
    for (int j = 0; j <= size2; j++)
        verif[0][j] = j;

    for (int i = 1; i <= size1; i++)
    {
        for (int j = 1; j <= size2; j++)
        {
            int cost = (a[j - 1] == b[i - 1]) ? 0 : 1;
            verif[i][j] = min(
                min(verif[i - 1][j] + 1, verif[i][j - 1] + 1),
                verif[i - 1][j - 1] + cost);
        }
    }

    return verif[size1][size2];
}

string Youtube::normalize_string(const string &input)
{
    string result = input;
    // convert to lowercase
    transform(result.begin(), result.end(), result.begin(), ::tolower);

    // remove special characters and extra whitespace
    static const regex cleanup_regex("[^a-z0-9]+");
    result = regex_replace(result, cleanup_regex, " ");

    // trim whitespace
    result = regex_replace(result, regex("^\\s+|\\s+$"), "");
    return result;
}

URL Youtube::get_music_url(SearchResult search_result)
{
    return "https://music.youtube.com/watch?v=" + search_result.video_id;
}

vector<URL> Youtube::search(AnyMetadata metadata)
{
    LOG_INFO("Creating youtube search query.", "Creating youtube search query");
    this->metadata = metadata;
    if (this->is_track())
    {
        TrackMetadata track = get<TrackMetadata>(metadata);
        Query track_query = this->query_builder.create_track_search_query(track);
        this->queries.push_back(track_query);
    }
    else if (this->is_album())
    {
        AlbumMetadata album_metadata = get<AlbumMetadata>(metadata);
        Query album_query = this->query_builder.create_album_search_query(album_metadata);
        this->queries.push_back(album_query);
    }
    else if (this->is_playlist())
    {
        PlaylistMetadata playlist = get<PlaylistMetadata>(metadata);

        for (const auto &track : playlist.tracks)
        {
            Query track_url = this->query_builder.create_track_search_query(track);
            this->queries.push_back(track_url);
        }
    }

    vector<URL> urls;

    for (const auto &query : this->queries)
    {
        string response = this->make_search_request(query);
        vector<SearchResult> search_results = this->parse_response(response);

        if (search_results.empty())
        {
            LOG_INFO("No results found", "No results found for query: " + query);
            continue;
        }

        const auto &best_match = search_results[0];
        if (best_match.score >= this->config.minimum_match_score)
        {
            urls.push_back(this->get_music_url(best_match));
            LOG_INFO("Found match.", "Found match for query" + query);
        }
        else
        {
            string base = "Skipping... Could not find a song that meets the minimum match score";
            LOG_INFO(base, base + "- Best match score too low (" + to_string(best_match.score) + ") for query: " + query);
        }
    }

    return urls;
}

