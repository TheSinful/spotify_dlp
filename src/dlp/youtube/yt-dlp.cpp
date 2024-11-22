#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#endif

#include <fstream>
#include <filesystem>
#include "yt-dlp.h"
#include "./data/yt-dlp-data.h"
#include "../../utils/logger.h"

YtDLP::YtDLP() : program_name(_WIN32 ? "yt_dlp.exe" : "yt_dlp")
{
    this->get_temp_path();

#ifdef _WIN32
    this->extract_yt_dlp_windows();
#else
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
    std::string temp_path = "/tmp/" + this->program_name; // Added missing slash
    this->yt_dlp_temp_path = std::filesystem::path(temp_path);
#endif
}

std::string YtDLP::get_path() const
{
    return this->yt_dlp_temp_path.string();
}

bool YtDLP::command_return_ok(YtDLPExitCodes code)
{
    switch (code)
    {
    case 0:
        return true;
    default:
        return false;
    }
}

CommandResult YtDLP::execute_command(const std::string &command)
{
    CommandResult result;
    std::array<char, 128> buffer;

    std::string cmd = command + " 2>&1";

#ifdef _WIN32
    FILE *pipe = _popen(cmd.c_str(), "r");
#else
    FILE *pipe = popen(cmd.c_str(), "r");
#endif

    if (!pipe)
    {
        std::string log = "Failed to execute command: " + command;
        THROW_AND_LOG(std::runtime_error, log, log);
    }

    // read out
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        result.out += buffer.data();
    }

#ifdef _WIN32
    result.cmd_exit_code = _pclose(pipe);
#else
    result.cmd_exit_code = pclose(pipe);
#endif

    // shift right by 8 to get actual exit code on windows
#ifdef _WIN32
    result.ytdlp_exit_code = static_cast<YtDLPExitCodes>(result.cmd_exit_code >> 8);
#else
    result.ytdlp_exit_code = static_cast<YtDLPErrorCodes>(WEXITSTATUS(result.cmd_exit_code));
#endif

    return result;
}

void YtDLP::download(DownloadConfig config)
{
    this->config = config;
}

std::string YtDLP::convert_download_file_type()
{
    switch (this->config.download_file_type)
    {
    case DownloadFileType::MP3:
        return "mp3";
    case DownloadFileType::M4A:
        return "m4a";
    case DownloadFileType::OPUS:
        return "opus";
    case DownloadFileType::VORBIS:
        return "vorbis";
    case DownloadFileType::WAV:
        return "wav";
    case DownloadFileType::BEST:
        return "best";
    case DownloadFileType::AAC:
        return "aac";
    case DownloadFileType::ALAC:
        return "alac";
    case DownloadFileType::FLAC:
        return "flac";
    default:
        return "best";
    }
}