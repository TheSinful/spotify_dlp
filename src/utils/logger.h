#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>
#include <string>
#include <fmt/core.h>

/*
   ! NOTE: DOCS WERE DONE BY CLAUDE BECAUSE I WAS TOO LAZY TO PROPERLY MAKE DOCUMENTATION FOR THIS,
   ! ALTHOUGH I WENT THRU EACH MANUALLY AND MADE SURE THEY WERE GOOD!
   ! I WANTED TO MAKE MAKE SURE THAT IS MADE CLEAR INCASE THERE ARE ERRORS I DID NOT PICK UP IN IT.
*/

/**
 * @brief Utility namespace for logging configuration and control
 *
 * The LoggerUtils namespace provides functionality to control logging behavior:
 * - Verbose mode: Switches between concise and detailed log messages
 * - Debug mode: Controls debug-level logging and sets appropriate log levels
 */

namespace LoggerUtils {
    namespace detail {
        static bool g_verbose = false;
        static bool g_debug = false;
    }

    // Changed return type to fmt::string_view
    static fmt::string_view select_message(
        fmt::string_view normal,
        fmt::string_view verbose)
    {
#ifdef FORCE_VERBOSE
        return verbose;
#else
        return detail::g_verbose ? verbose : normal;
#endif
    }

    inline void setVerbose(bool v) noexcept { detail::g_verbose = v; }
    inline void setDebug(bool d) noexcept {
        detail::g_debug = d;
        spdlog::set_level(d ? spdlog::level::debug : spdlog::level::info);
    }
}

/**
 * @brief Log an error message with normal and verbose variants
 * @param normal_msg Concise message for normal mode
 * @param verbose_msg Detailed message for verbose mode
 * @param ... Optional format arguments
 */
#define LOG_ERROR(normal_msg, verbose_msg, ...)                                 \
    do                                                                          \
    {                                                                           \
        spdlog::error(                                                          \
            fmt::runtime(LoggerUtils::select_message(normal_msg, verbose_msg)), \
            ##__VA_ARGS__);                                                     \
    } while (0)

/**
 * @brief Log a warning message with normal and verbose variants
 * @param normal_msg Concise message for normal mode
 * @param verbose_msg Detailed message for verbose mode
 * @param ... Optional format arguments
 */
#define LOG_WARN(normal_msg, verbose_msg, ...)                                  \
    do                                                                          \
    {                                                                           \
        spdlog::warn(                                                           \
            fmt::runtime(LoggerUtils::select_message(normal_msg, verbose_msg)), \
            ##__VA_ARGS__);                                                     \
    } while (0)

/**
 * @brief Log an info message with normal and verbose variants
 * @param normal_msg Concise message for normal mode
 * @param verbose_msg Detailed message for verbose mode
 * @param ... Optional format arguments
 */
#define LOG_INFO(normal_msg, verbose_msg, ...)                                  \
    do                                                                          \
    {                                                                           \
        spdlog::info(                                                           \
            fmt::runtime(LoggerUtils::select_message(normal_msg, verbose_msg)), \
            ##__VA_ARGS__);                                                     \
    } while (0)

/**
 * @brief Log a debug message when debug mode is enabled
 * @param normal_msg Concise message for normal mode
 * @param verbose_msg Detailed message for verbose mode
 * @param ... Optional format arguments
 * @note Only logs when LoggerUtils::isDebug() is true
 */
#define LOG_DEBUG(normal_msg, verbose_msg, ...)                                     \
    do                                                                              \
    {                                                                               \
        if (LoggerUtils::detail::g_debug)                                           \
        {                                                                           \
            spdlog::debug(                                                          \
                fmt::runtime(LoggerUtils::select_message(normal_msg, verbose_msg)), \
                ##__VA_ARGS__);                                                     \
        }                                                                           \
    } while (0)

/**
 * @brief Log a trace message when debug mode is enabled
 * @param normal_msg Concise message for normal mode
 * @param verbose_msg Detailed message for verbose mode
 * @param ... Optional format arguments
 * @note Only logs when LoggerUtils::isDebug() is true
 */
#define LOG_TRACE(normal_msg, verbose_msg, ...)                                     \
    do                                                                              \
    {                                                                               \
        if (LoggerUtils::detail::g_debug)                                           \
        {                                                                           \
            spdlog::trace(                                                          \
                fmt::runtime(LoggerUtils::select_message(normal_msg, verbose_msg)), \
                ##__VA_ARGS__);                                                     \
        }                                                                           \
    } while (0)

/**
 * @brief Log an error and throw an exception with the same message
 * @param exception_type The type of exception to throw
 * @param normal_msg Concise message for normal mode
 * @param verbose_msg Detailed message for verbose mode
 * @param ... Optional format arguments
 * @throws exception_type with the appropriate message
 */
#define THROW_AND_LOG(exception_type, normal_msg, verbose_msg, ...)             \
    do                                                                          \
    {                                                                           \
        std::string msg = fmt::format(                                          \
            fmt::runtime(LoggerUtils::select_message(normal_msg, verbose_msg)), \
            ##__VA_ARGS__);                                                     \
        spdlog::error(msg);                                                     \
        throw exception_type(msg);                                              \
    } while (0)

#endif