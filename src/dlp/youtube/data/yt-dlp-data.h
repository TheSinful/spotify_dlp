#pragma once
#ifndef YT_DLP_DATA_H
#define YT_DLP_DATA_H

namespace YtDLPData
{
#ifdef _WIN32
    extern const unsigned char yt_dlp_windows[];
    extern const size_t yt_dlp_windows_size;
#else
    extern const unsigned char yt_dlp_linux[];
    extern const size_t yt_dlp_linux_size;
#endif
}

#endif