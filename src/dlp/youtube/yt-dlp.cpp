#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

#include <fstream>
#include <filesystem>
#include "yt-dlp.h"
#include "./data/yt-dlp-data.h"
#include "../../utils/logger.h"

YtDLP::YtDLP()
{
#ifdef _WIN32
    this->program_name = "yt_dlp.exe";
    this->extract_yt_dlp_windows();
#else
    this->program_name = "yt_dlp";
    this->extract_yt_dlp_linux();
#endif
}

void YtDLP::extract_yt_dlp_linux()
{
#ifdef __linux__
    std::ofstream outfile(this->yt_dlp_temp_path, std::ios::binary);
    if (!outfile)
    {
        std::string log = "Failed to create tempfile for YTDlp (linux).";
        THROW_AND_LOG(std::runtime_error, log, log);
    }

    outfile.write(reinterpret_cast<const char *>(YtDLPData::yt_dlp_linux), YtDLPData::yt_dlp_linux_size);
#endif
}

void YtDLP::extract_yt_dlp_windows()
{
    std::ofstream outfile(this->yt_dlp_temp_path, std::ios::binary);
    if (!outfile)
    {
        std::string log = "Failed to create tempfile for YTDlp (windows).";
        THROW_AND_LOG(std::runtime_error, log, log);
    }

    outfile.write(reinterpret_cast<const char *>(YtDLPData::yt_dlp_windows), YtDLPData::yt_dlp_windows_size);
    outfile.close();
}

void YtDLP::get_temp_path()
{
#ifdef _WIN32
    wchar_t temp_dir[MAX_PATH];
    GetTempPathW(MAX_PATH, temp_dir);
    this->yt_dlp_temp_path = std::filesystem::path(temp_dir) / this->program_name;
#else
    std::string temp_path = "/tmp" + this->program_name;
    this->yt_dlp_temp_path = std::filesystem::path(temp_path);
#endif
}

std::string YtDLP::get_path() const
{
    return this->yt_dlp_temp_path.string();
}