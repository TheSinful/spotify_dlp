#include "spotify-dlp.h"
#include <filesystem>
#include <string>

DownloadConfig dlp_create_default_download_config()
{
    DownloadConfig config;

    config.download_file_type = BEST;
    config.retries = 10;
    config.audio_quality = 10;
    config.output = NULL;
    config.minimum_match_score = 0.7;
    config.error = OK;

    return config;
}

DownloadConfig dlp_create_default_config_with_download_path(const std::string &path)
{
    DownloadConfig config;
    config.download_file_type = BEST;
    config.retries = 10;
    config.audio_quality = 10;
    config.output = NULL;
    config.minimum_match_score = 0.7;
    config.error = OK;

    if (path.empty())
        return config;

    if (std::filesystem::is_directory(path))
    {
        config.error = ERROR_DIRECTORY_PATH_NOT_ALLOWED;
        return config;
    }

    size_t dot_pos = path.find_last_of('.');
    std::string new_path;

    if (dot_pos != std::string::npos)
    {
        // replace extension
        new_path = path.substr(0, dot_pos) + ".%(ext)s";
    }
    else
    {
        // no extension, append %(ext)s
        new_path = path + ".%(ext)s";
    }

    char *output_str = (char *)malloc(new_path.length() + 1);
    if (output_str)
    {
        strcpy(output_str, new_path.c_str());
        config.output = output_str;
    }

    return config;
}
