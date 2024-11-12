#pragma once
#ifndef CURL_UTILS_H
#define CURL_UTILS_H

#include <curl/curl.h>
#include <string>

// Callback function for CURL write operations
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

class CurlException : public std::exception
{
    std::string msg;

public:
    CurlException(CURLcode code) : msg(curl_easy_strerror(code)) {}
    CurlException(const std::string &error_msg) : msg(error_msg) {}
    const char *what() const noexcept override { return msg.c_str(); }
};

// RAII wrapper for CURL handles
// curl_utils.h
class CurlGuard
{
public:
    CurlGuard();
    ~CurlGuard();

    CurlGuard(const CurlGuard&) = delete;
    CurlGuard& operator=(const CurlGuard&) = delete;
    CurlGuard(CurlGuard&&) = delete;
    CurlGuard& operator=(CurlGuard&&) = delete;

    void set_headers(struct curl_slist *new_headers); 

    CURL *get(); 

private:
    CURL *curl = nullptr;
    struct curl_slist *headers = nullptr;
};

#endif