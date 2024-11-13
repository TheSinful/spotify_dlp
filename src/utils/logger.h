#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <spdlog/spdlog.h>

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
    /** @brief Controls whether verbose (detailed) messages are used */
    inline bool g_verbose = false;
    
    /** @brief Controls whether debug messages are logged */
    inline bool g_debug = false;
    
    /**
     * @brief Enable or disable verbose logging
     * @param verbose True to enable verbose messages, false for concise messages
     */
    inline void setVerbose(bool verbose) { g_verbose = verbose; }
    
    /** @brief Check if verbose logging is enabled */
    inline bool isVerbose() { return g_verbose; }
    
    /**
     * @brief Enable or disable debug logging
     * @param debug True to enable debug logging, false to disable
     * 
     * When enabled:
     * - Sets spdlog level to debug
     * - Allows LOG_DEBUG and LOG_TRACE macros to output
     * When disabled:
     * - Sets spdlog level to info
     * - Suppresses LOG_DEBUG and LOG_TRACE output
     */
    inline void setDebug(bool debug) {
        g_debug = debug;
        if (debug) {
            spdlog::set_level(spdlog::level::debug);
        } else {
            spdlog::set_level(spdlog::level::info);
        }
    }
    
    /** @brief Check if debug logging is enabled */
    inline bool isDebug() { return g_debug; }
}

/**
 * @brief Log an error message with normal and verbose variants
 * @param normal_msg Concise message for normal mode
 * @param verbose_msg Detailed message for verbose mode
 * @param ... Optional format arguments
 */
#define LOG_ERROR(normal_msg, verbose_msg, ...) \
    spdlog::error(LoggerUtils::g_verbose ? verbose_msg : normal_msg, ##__VA_ARGS__)

/**
 * @brief Log a warning message with normal and verbose variants
 * @param normal_msg Concise message for normal mode
 * @param verbose_msg Detailed message for verbose mode
 * @param ... Optional format arguments
 */
#define LOG_WARN(normal_msg, verbose_msg, ...) \
    spdlog::warn(LoggerUtils::g_verbose ? verbose_msg : normal_msg, ##__VA_ARGS__)

/**
 * @brief Log an info message with normal and verbose variants
 * @param normal_msg Concise message for normal mode
 * @param verbose_msg Detailed message for verbose mode
 * @param ... Optional format arguments
 */
#define LOG_INFO(normal_msg, verbose_msg, ...) \
    spdlog::info(LoggerUtils::g_verbose ? verbose_msg : normal_msg, ##__VA_ARGS__)

/**
 * @brief Log a debug message when debug mode is enabled
 * @param normal_msg Concise message for normal mode
 * @param verbose_msg Detailed message for verbose mode
 * @param ... Optional format arguments
 * @note Only logs when LoggerUtils::g_debug is true
 */
#define LOG_DEBUG(normal_msg, verbose_msg, ...) \
    do { if (LoggerUtils::g_debug) { \
        spdlog::debug(LoggerUtils::g_verbose ? verbose_msg : normal_msg, ##__VA_ARGS__); \
    } } while(0)

/**
 * @brief Log a trace message when debug mode is enabled
 * @param normal_msg Concise message for normal mode
 * @param verbose_msg Detailed message for verbose mode
 * @param ... Optional format arguments
 * @note Only logs when LoggerUtils::g_debug is true
 */
#define LOG_TRACE(normal_msg, verbose_msg, ...) \
    do { if (LoggerUtils::g_debug) { \
        spdlog::trace(LoggerUtils::g_verbose ? verbose_msg : normal_msg, ##__VA_ARGS__); \
    } } while(0)

/**
 * @brief Log an error and throw an exception with the same message
 * @param exception_type The type of exception to throw
 * @param normal_msg Concise message for normal mode
 * @param verbose_msg Detailed message for verbose mode
 * @param ... Optional format arguments
 * @throws exception_type with the appropriate message
 */
#define THROW_WITH_LOG(exception_type, normal_msg, verbose_msg, ...) \
    do { \
        spdlog::error(LoggerUtils::g_verbose ? verbose_msg : normal_msg, ##__VA_ARGS__); \
        throw exception_type(LoggerUtils::g_verbose ? verbose_msg : normal_msg); \
    } while (0)

#endif