#include <stdexcept>
#include "youtube.h"
#include "../../utils/curl_utils.h"
#include "../../utils/logger.h"

Youtube::Youtube(AnyMetadata metadata, std::string yt_api_key)
    : metadata(metadata), url_builder(), queries(), api_key(yt_api_key)
{
    LOG_INFO("Creating youtube search query.", "Creating youtube search query");
    if (this->is_track())
    if (this->is_track())
    {
        TrackMetadata track = std::get<TrackMetadata>(this->metadata);
        Query track_query = this->url_builder.create_track_search_query(track);
        this->queries.push_back(track_query);
    }
    else if (this->is_album())
    {
        AlbumMetadata album_metadata = std::get<AlbumMetadata>(this->metadata);
        Query album_query = this->url_builder.create_album_search_query(album_metadata);
        this->queries.push_back(album_query);
    }
    else if (this->is_playlist())
    {
        PlaylistMetadata playlist = std::get<PlaylistMetadata>(this->metadata);

        for (const auto &track : playlist.tracks)
        {
            Query track_url = this->url_builder.create_track_search_query(track);
            this->queries.push_back(track_url);
        }
    }
};

bool Youtube::is_track()
{
    return std::holds_alternative<TrackMetadata>(this->metadata);
}

bool Youtube::is_album()
{
    return std::holds_alternative<AlbumMetadata>(this->metadata);
}

bool Youtube::is_playlist()
{
    return std::holds_alternative<PlaylistMetadata>(this->metadata);
}

std::string Youtube::make_search_request(const std::string &query)
{
    LOG_INFO("Making search request", "Making search request for query" + query);
    std::string response;

    CurlGuard curl;

    struct curl_slist *headers = nullptr;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl.set_headers(headers);
    std::string header_log = "Created search request headers";
    LOG_INFO(header_log, header_log);

    std::string url = "https://youtube.googleapis.com/youtube/v3/search"
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
    std::string body_log = "Created search request body, making request!";
    LOG_INFO(body_log, body_log);

    CURLcode res = curl_easy_perform(curl.get());
    if (res != CURLE_OK)
    {
        THROW_AND_LOG(CurlException, "Response returned not ok", "Response from Youtube API returned not ok" + res);
    }

    return response;
}

std::string Youtube::url_encode(const std::string &decoded)
{
    const auto encoded_value = curl_easy_escape(nullptr, decoded.c_str(), static_cast<int>(decoded.length()));
    std::string result(encoded_value);
    curl_free(encoded_value);
    return result;
}

std::vector<SearchResult> Youtube::parse_response(const std::string &response)
{
    std::vector<SearchResult> results;
    auto json = nlohmann::json::parse(response);

    if (!json.contains("items"))
    {
        THROW_AND_LOG(std::runtime_error, "Failed to find items in response", "Failed to find items in response from Youtube API ");
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
    std::sort(results.begin(), results.end(),
              [](const SearchResult &a, const SearchResult &b)
              {
                  return a.score > b.score;
              });

    return results;
}

double Youtube::calculate_match_score(const SearchResult &result)
{
    if (is_track())
    {
        const auto &track = std::get<TrackMetadata>(metadata);
        double score = 0.0;

        // Title match (45%)
        double title_score = calculate_title_similarity(
            normalize_string(result.title),
            normalize_string(track.name));
        score += title_score * 0.45;

        // Artist match (45%)
        std::string track_artists = normalize_string(url_builder.join_artists(track.artists));
        std::string result_artists = normalize_string(result.artist);
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
    else if (is_album())
    {
        const auto &album = std::get<AlbumMetadata>(metadata);
        double score = 0.0;

        // Album title match (50%)
        double title_score = calculate_title_similarity(
            normalize_string(result.title),
            normalize_string(album.name));
        score += title_score * 0.50;

        // Artist match (40%)
        std::string album_artists = normalize_string(url_builder.join_artists(album.artists));
        std::string result_artists = normalize_string(result.artist);
        double artist_score = calculate_title_similarity(album_artists, result_artists);
        score += artist_score * 0.40;

        // Additional scoring for keywords suggesting it's an album (10%)
        std::string normalized_title = normalize_string(result.title);
        if (normalized_title.find("album") != std::string::npos ||
            normalized_title.find("full album") != std::string::npos)
        {
            score += 0.10;
        }

        return score;
    }

    return 0.0; // Playlists are handled as individual tracks
}

double Youtube::calculate_title_similarity(const std::string &a, const std::string &b)
{
    const auto max_len = std::max(a.length(), b.length());
    if (max_len == 0)
        return 0.0;

    const auto distance = levenshtein_distance(a, b);
    return 1.0 - (static_cast<double>(distance) / max_len);
}

int Youtube::levenshtein_distance(std::string a, std::string b)
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
            verif[i][j] = std::min(
                std::min(verif[i - 1][j] + 1, verif[i][j - 1] + 1),
                verif[i - 1][j - 1] + cost);
        }
    }

    return verif[size1][size2];
}

std::string Youtube::normalize_string(const std::string &input)
{
    std::string result = input;
    // convert to lowercase
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);

    // remove special characters and extra whitespace
    static const std::regex cleanup_regex("[^a-z0-9]+");
    result = std::regex_replace(result, cleanup_regex, " ");

    // trim whitespace
    result = std::regex_replace(result, std::regex("^\\s+|\\s+$"), "");
    return result;
}
