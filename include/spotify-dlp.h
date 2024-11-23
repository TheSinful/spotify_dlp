#ifndef SPOTIFY_DLP_H
#define SPOTIFY_DLP_H

#ifdef __cplusplus
extern "C"
{
#endif

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
    } DownloadConfig;

    DownloadConfig dlp_create_default_download_config(void);

#ifdef __cplusplus
}
#endif

static inline DownloadConfig dlp_create_default_download_config(void)
{
    DownloadConfig config = {
        .download_file_type = BEST,
        .retries = 10,
        .audio_quality = 10,
        .output = NULL,
        .minimum_match_score = 0.7};
    return config;
}

#endif // SPOTIFY_DLP_H
