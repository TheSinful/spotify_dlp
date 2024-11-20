
extern "C" typedef enum {
    MP3,    // --extract-audio --audio-format mp3
    M4A,    // --extract-audio --audio-format m4a
    OPUS,   // --extract-audio --audio-format opus
    VORBIS, // --extract-audio --audio-format vorbis
    WAV,    // --extract-audio --audio-format wav
    BEST,   // --extract-audio --audio-format best
} DownloadFileType;

extern "C" typedef struct
{
    DownloadFileType download_file_type;
    int audio_quality; // 0 (best) to 10 (worst) for lossy formats
    double minimum_match_score; // minimum score for a song to be a match 
} DownloadConfig;

