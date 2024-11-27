#ifndef SPOTIFY_DLP_H
#define SPOTIFY_DLP_H

#include <string>
#include <filesystem>

#ifdef __cplusplus
extern "C"
{
#endif
    typedef enum
    {
        OK,
        ERROR_DIRECTORY_PATH_NOT_ALLOWED
    } Return;

    typedef enum
    {
        MP3,
        M4A,
        OPUS,
        VORBIS,
        WAV,
        BEST,
        AAC,
        ALAC,
        FLAC,
    } DownloadFileType;

    typedef struct DownloadConfig
    {

        // note: -x is set on by default

        // yt-dlp flags
        // File type for audio extraction
        // Flag: --extract-audio --audio-format FORMAT
        // where FORMAT = mp3|m4a|opus|vorbis|wav|best|aac|alac|flac
        DownloadFileType download_file_type;

        // Number of download retries on failure
        // Flag: --retries N
        // Default: 10, -1 means use default
        int retries;

        // Audio quality for transcoding
        // Flag: --audio-quality QUALITY
        // Range: 0-10 (0=best, 10=worst)
        // Default: 10, -1 means use default
        int audio_quality;

        // Output template for downloaded files
        // Flag: -o/--output TEMPLATE
        // Example: "%(title)s.%(ext)s"
        // NULL means use yt-dlp default template
        // ! TODO: make sure this pointer is handled elsewhere, i'm not sure where it's meant to be handled but I assume in each implementation/binding
        const char *output;

        // algorithim options
        double minimum_match_score; // 0.0 - 1.0 default 0.7

        Return error;
    } DownloadConfig;

    /**
     * Creates a default DownloadConfig.
     *
     * @return DownloadConfig config = {
                .download_file_type = BEST,
                .retries = 10,
                .audio_quality = 10,
                .output = NULL,
                .minimum_match_score = 0.7,
                .error = OK};
     */
    static inline DownloadConfig dlp_create_default_download_config()
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

    /**
     * Creates a default config with custom output path formatting.
     *
     * Takes a file path and automatically formats it for yt-dlp by replacing or adding
     * the %(ext)s template variable.
     *
     * @param path      Path where the file should be downloaded.
     *                  - If path includes extension (e.g. ".mp3"), it will be replaced with %(ext)s
     *                  - If no extension, %(ext)s will be appended
     *                  - If NULL, uses yt-dlp default output template
     *
     * @return DownloadConfig with default values and formatted output path
     *
     * @example
     * const char* path1 = "C:\\Music\\Song.mp3";     // Becomes: C:\Music\Song.%(ext)s
     * const char* path2 = "C:\\Music\\Song";         // Becomes: C:\Music\Song.%(ext)s
     * DownloadConfig config = dlp_create_default_config_with_download_path(path1);
     */
    static inline DownloadConfig dlp_create_default_config_with_download_path(const char *path)
    {
        DownloadConfig config;
        config.download_file_type = BEST;
        config.retries = 10;
        config.audio_quality = 10;
        config.output = nullptr;
        config.minimum_match_score = 0.7;
        config.error = OK;

        if (path == nullptr || strlen(path) == 0)
            return config;

        if (std::filesystem::is_directory(path))
        {
            config.error = ERROR_DIRECTORY_PATH_NOT_ALLOWED;
            return config;
        }

        const char *dot_pos = strrchr(path, '.');
        char *new_path;
        size_t path_len = strlen(path);

        if (dot_pos != nullptr)
        {
            size_t base_len = dot_pos - path;
            new_path = new char[base_len + 9]; // +9 for ".%(ext)s\0"
            strncpy(new_path, path, base_len);
            strcpy(new_path + base_len, ".%(ext)s");
        }
        else
        {
            new_path = new char[path_len + 9]; // +9 for ".%(ext)s\0"
            strcpy(new_path, path);
            strcat(new_path, ".%(ext)s");
        }

        config.output = new_path;
        return config;
    }

    static inline Return download(DownloadConfig config, const char *url, const char *spotify_client_id, const char *spotify_client_secret, const char *youtube_api_key)
    {
        

        return OK;
    }

#ifdef __cplusplus
}
#endif

#endif // SPOTIFY_DLP_H
