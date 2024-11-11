#pragma once
#ifndef CURL_UTILS_H
#define CURL_UTILS_H

#include <curl/curl.h>
#include <string>

// Callback function for CURL write operations
size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);

// RAII wrapper for CURL handles
class CurlGuard {
private:
    CURL *curl;
    curl_slist *headers;

public:
    CurlGuard();
    ~CurlGuard();
    
    void set_headers(curl_slist *h);
    CURL *get();
};

#endif 