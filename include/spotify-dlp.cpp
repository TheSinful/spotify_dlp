#include "spotify-dlp.h"
#include <string.h>
#include <stdlib.h>
#include <filesystem>

DownloadConfig dlp_create_default_download_config(void)
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

DownloadConfig dlp_create_default_config_with_download_path(const char *path)
{
    DownloadConfig config;
    config.download_file_type = BEST;
    config.retries = 10;
    config.audio_quality = 10;
    config.output = NULL;
    config.minimum_match_score = 0.7;
    config.error = OK;

    if (!path)
        return config;

    if (std::filesystem::is_directory(path))
    {
        config.error = ERROR_DIRECTORY_PATH_NOT_ALLOWED;
        return config;
    }

    const char *dot = strrchr(path, '.');
    if (dot)
    {
        // replace extension
        size_t base_len = dot - path;
        char *new_path = (char *)malloc(base_len + 10); // +10 for "%(ext)s\0"
        if (new_path)
        {
            strncpy(new_path, path, base_len);
            strcat(new_path, ".%(ext)s");
            config.output = new_path;
        }
    }
    else
    {
        // no extension %(ext)s
        size_t len = strlen(path);
        char *new_path = (char *)malloc(len + 10);
        if (new_path)
        {
            strcpy(new_path, path);
            strcat(new_path, ".%(ext)s");
            config.output = new_path;
        }
    }

    return config;
}
