#ifndef SPOTIFY_DLP_H
#define SPOTIFY_DLP_H

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
        MP3,    // --extract-audio  mp3
        M4A,    // --extract-audio  m4a
        OPUS,   // --extract-audio  opus
        VORBIS, // --extract-audio  vorbis
        WAV,    // --extract-audio  wav
        BEST,   // --extract-audio  best
        AAC,    // --extract-audio  aac
        ALAC,   // --extract-audio  alac
        FLAC,   // --extract-audio  flac
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
    static inline DownloadConfig dlp_create_default_download_config(void);

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
    static inline DownloadConfig dlp_create_default_config_with_download_path(const char *path);

#ifdef __cplusplus
}
#endif

#endif // SPOTIFY_DLP_H
