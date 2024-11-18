#include "search_builder.h"

URL YoutubeSearchBuilder::create_track_search_url(const TrackMetadata &track)
{
    std::string artistString = this->join_artists(track.artists);

    std::string query = this->sanitize_query(
        artistString + " - " +
        track.name +
        (track.album_name.has_value() ? " " + track.album_name.value() : "") +
        " official audio");

    return "https://www.youtube.com/results?search_query=" + query;
}

std::string YoutubeSearchBuilder::sanitize_query(const std::string &input)
{
    std::string result = std::regex_replace(input, std::regex("\\s+"), "+");

    result = std::regex_replace(result, std::regex("[^a-zA-Z0-9+]"), "");

    return this->url_encode(result);
}

URL YoutubeSearchBuilder::create_album_search_url(const AlbumMetadata &album)
{
    std::string artistString = this->join_artists(album.artists);

    std::string query = this->sanitize_query(
        artistString + " " +
        album.name +
        " full album " +
        album.release_date.substr(0, 4));

    return "https://www.youtube.com/results?search_query=" + query;
}

URL YoutubeSearchBuilder::url_encode(const std::string &input)
{
    std::string result;
    for (char c : input)
    {
        if (isalnum(c) || c == '+' || c == '-' || c == '_' || c == '.')
        {
            result += c;
        }
        else
        {
            result += '%' + std::string(2, '0' + (c >> 4));
            result += std::string(1, "0123456789ABCDEF"[c & 0xF]);
        }
    }
    return result;
}

std::string YoutubeSearchBuilder::join_artists(const std::vector<Artist> &artists)
{
    if (artists.empty())
        return "";

    std::stringstream ss;
    ss << artists[0].name; // Primary artist

    if (artists.size() > 1)
    {
        ss << " feat.";
        for (size_t i = 1; i < artists.size(); ++i)
        {
            if (i == artists.size() - 1)
            {
                ss << " " << artists[i].name;
            }
            else
            {
                ss << " " << artists[i].name << ",";
            }
        }
    }
    return ss.str();
}