#include "util/curl.hpp"
#include "util/config.hpp"
#include <curl/curl.h>
#include <string>
#include <sstream>
#include <iostream>

static size_t writeDataFile(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

size_t writeDataBuffer(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::ostringstream *stream = (std::ostringstream*)userdata;
    size_t count = size * nmemb;
    stream->write(ptr, count);
    return count;
}

namespace inst::curl {
    bool downloadFile (const std::string ourUrl, const char *pagefilename) {
        CURL *curl_handle;
        CURLcode result;
        FILE *pagefile;
        
        curl_global_init(CURL_GLOBAL_ALL);
        curl_handle = curl_easy_init();
        
        curl_easy_setopt(curl_handle, CURLOPT_URL, ourUrl.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 1000L);
        curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, 1000L);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeDataFile);
        
        pagefile = fopen(pagefilename, "wb");
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
        result = curl_easy_perform(curl_handle);
        
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        fclose(pagefile);

        if (result == CURLE_OK) return true;
        else {
            printf(curl_easy_strerror(result));
            return false;
        }
    }

    std::string downloadToBuffer (const std::string ourUrl) {
        CURL *curl_handle;
        CURLcode result;
        std::ostringstream stream;
        
        curl_global_init(CURL_GLOBAL_ALL);
        curl_handle = curl_easy_init();
        
        curl_easy_setopt(curl_handle, CURLOPT_URL, ourUrl.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 1000L);
        curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, 1000L);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeDataBuffer);
        
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &stream);
        result = curl_easy_perform(curl_handle);
        
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();

        if (result == CURLE_OK) return stream.str();
        else {
            printf(curl_easy_strerror(result));
            return "";
        }
    }
}