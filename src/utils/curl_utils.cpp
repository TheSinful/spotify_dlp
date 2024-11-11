#include "curl_utils.h"
#include <stdexcept>

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

CurlGuard::CurlGuard() : headers(nullptr)
{
    this->curl = curl_easy_init();
    if (!curl)
    {
        throw std::runtime_error("Failed to initialize CURL!");
    };
}

CurlGuard::~CurlGuard()
{
    if (headers)
        curl_slist_free_all(headers);
    if (curl)
        curl_easy_cleanup(curl);
}

void CurlGuard::set_headers(curl_slist *h) { headers = h; }
CURL *CurlGuard::get() { return curl; }