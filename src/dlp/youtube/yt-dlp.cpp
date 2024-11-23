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

using namespace std;

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
    ofstream outfile(this->yt_dlp_temp_path, ios::binary);
    if (!outfile)
    {
        string log = "Failed to create tempfile for YTDlp (linux).";
        THROW_AND_LOG(runtime_error, log, log);
    }

    outfile.write(reinterpret_cast<const char *>(YtDLPData::yt_dlp_linux), YtDLPData::yt_dlp_linux_size);
#endif
}

void YtDLP::extract_yt_dlp_windows()
{
    ofstream outfile(this->yt_dlp_temp_path, ios::binary);
    if (!outfile)
    {
        string log = "Failed to create tempfile for YTDlp (windows).";
        THROW_AND_LOG(runtime_error, log, log);
    }

    outfile.write(reinterpret_cast<const char *>(YtDLPData::yt_dlp_windows), YtDLPData::yt_dlp_windows_size);
    outfile.close();
}

void YtDLP::get_temp_path()
{
#ifdef _WIN32
    wchar_t temp_dir[MAX_PATH];
    GetTempPathW(MAX_PATH, temp_dir);
    this->yt_dlp_temp_path = filesystem::path(temp_dir) / this->program_name;
#else
    string temp_path = "/tmp/" + this->program_name;
    h this->yt_dlp_temp_path = filesystem::path(temp_path);
#endif
}

string YtDLP::get_path() const
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

CommandResult YtDLP::execute_command(const string &command)
{
    CommandResult result;
    array<char, 128> buffer;

#ifdef _WIN32
    // Windows needs temporary files to handle separate streams
    string cmd = command + " 1>stdout.tmp 2>stderr.tmp";
    result.cmd_exit_code = system(cmd.c_str()); // Capture the return value

    ifstream stdout_file("stdout.tmp");
    string line;
    while (getline(stdout_file, line))
    {
        LOG_INFO(line, line);
        result.out += line + "\n";
    }

    ifstream stderr_file("stderr.tmp");
    while (getline(stderr_file, line))
    {
        LOG_ERROR(line, line);
        result.err += line + "\n";
    }

    stdout_file.close();
    stderr_file.close();
    remove("stdout.tmp");
    remove("stderr.tmp");

    result.cmd_exit_code = result.cmd_exit_code;
    result.ytdlp_exit_code = static_cast<YtDLPExitCodes>(result.cmd_exit_code);
#else
    // On Linux we can use popen twice to read both streams
    FILE *stdout_pipe = popen((command + " 2>/dev/null").c_str(), "r");
    FILE *stderr_pipe = popen((command + " 2>&1 1>/dev/null").c_str(), "r");

    if (!stdout_pipe || !stderr_pipe)
    {
        string log = "Failed to execute command: " + command;
        THROW_AND_LOG(runtime_error, log, log);
    }

    // Read stdout
    while (fgets(buffer.data(), buffer.size(), stdout_pipe) != nullptr)
    {
        string line = buffer.data();
        if (!line.empty() && line[line.length() - 1] == '\n')
        {
            line.pop_back();
        }
        LOG_INFO(line);
        result.out += line + "\n";
    }

    // Read stderr
    while (fgets(buffer.data(), buffer.size(), stderr_pipe) != nullptr)
    {
        string line = buffer.data();
        if (!line.empty() && line[line.length() - 1] == '\n')
        {
            line.pop_back();
        }
        LOG_ERROR(line);
        result.err += line + "\n";
    }

    result.cmd_exit_code = pclose(stdout_pipe);
    pclose(stderr_pipe);
    result.ytdlp_exit_code = static_cast<YtDLPExitCodes>(WEXITSTATUS(result.cmd_exit_code));
#endif

    return result;
}

void YtDLP::download(DownloadConfig config, const string &url)
{
    this->config = config;

    string command = this->get_path();
    command += " \"" + url + "\" -x"; // Quote the URL
    command += " --audio-format " + this->get_download_file_type();

    if (config.retries >= 0)
    {
        command += " --retries " + to_string(config.retries);
    }

    if (config.audio_quality >= 0)
    {
        command += " --audio-quality " + to_string(config.audio_quality);
    }

    if (config.output)
    {
        command += " -o \"" + string(config.output) + "\""; // Quote the output template
    }

    CommandResult result = this->execute_command(command);

    // If there's an error message or non-zero exit code, throw an exception
    if (!result.err.empty() || result.cmd_exit_code != 0)
    {
        string log = "Failed to download: " + url;
        string error_details = result.err.empty() ? "" : ": " + result.err;
        THROW_AND_LOG(runtime_error, log, log + error_details);
    }

    string log;
    string error_details = result.err.empty() ? "" : ": " + result.err;

    switch (result.ytdlp_exit_code)
    {
    case YtDLPExitCodes::Success:
        return;
    case YtDLPExitCodes::UpdateRequired:
        log = "YT-DLP update required!";
        THROW_AND_LOG(runtime_error, log, log + error_details);
    case YtDLPExitCodes::CancelledByMaxDownloads:
        log = "Reached set max downloads.";
        THROW_AND_LOG(runtime_error, log, log + error_details);
    case YtDLPExitCodes::UserOptionsError:
        log = "Error in user options!";
        THROW_AND_LOG(runtime_error, log, log + " when attempting to download " + url + error_details);
    case YtDLPExitCodes::GenericError:
        log = "Generic YT-DLP error occurred!";
        THROW_AND_LOG(runtime_error, log, log + " when attempting to download " + url + error_details);
    default:
        log = "Unknown error occurred!";
        THROW_AND_LOG(runtime_error, log, log + " Exit code: " + to_string(result.ytdlp_exit_code) + error_details);
    }
}

string YtDLP::get_download_file_type()
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