#include "curl_utils.h"
#include <stdexcept>

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

CurlGuard::CurlGuard()
{
    curl = curl_easy_init();
    if (!curl)
    {
        throw CurlException("Failed to initialize CURL handle");
    }
}

CurlGuard::~CurlGuard()
{
    if (headers)
    {
        curl_slist_free_all(headers);
        headers = nullptr;
    }
    if (curl)
    {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

void CurlGuard::set_headers(struct curl_slist *new_headers)
{
    if (headers)
    {
        curl_slist_free_all(headers);
    }
    headers = new_headers;
}

CURL *CurlGuard::get() { return curl; }