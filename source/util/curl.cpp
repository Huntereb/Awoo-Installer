#include <curl/curl.h>
#include <string>
#include <sstream>
#include <iostream>
#include "util/curl.hpp"
#include "util/config.hpp"
#include "util/error.hpp"
#include "sdInstall.hpp"

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

int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
    if (ultotal) {
        int uploadProgress = (int)(((double)ulnow / (double)ultotal) * 100.0);
        inst::ui::setInstBarPerc(uploadProgress);
    } else if (dltotal) {
        int downloadProgress = (int)(((double)dlnow / (double)dltotal) * 100.0);
        inst::ui::setInstBarPerc(downloadProgress);
    }
    return 0;
}

namespace inst::curl {
    bool downloadFile (const std::string ourUrl, const char *pagefilename, long timeout, bool writeProgress) {
        CURL *curl_handle;
        CURLcode result;
        FILE *pagefile;
        
        curl_global_init(CURL_GLOBAL_ALL);
        curl_handle = curl_easy_init();
        
        curl_easy_setopt(curl_handle, CURLOPT_URL, ourUrl.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Awoo-Installer");
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, timeout);
        curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, timeout);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeDataFile);
        if (writeProgress) curl_easy_setopt(curl_handle, CURLOPT_XFERINFOFUNCTION, progress_callback);
        
        pagefile = fopen(pagefilename, "wb");
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
        result = curl_easy_perform(curl_handle);
        
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        fclose(pagefile);

        if (result == CURLE_OK) return true;
        else {
            LOG_DEBUG(curl_easy_strerror(result));
            return false;
        }
    }

    std::string downloadToBuffer (const std::string ourUrl, int firstRange, int secondRange, long timeout) {
        CURL *curl_handle;
        CURLcode result;
        std::ostringstream stream;
        
        curl_global_init(CURL_GLOBAL_ALL);
        curl_handle = curl_easy_init();
        
        curl_easy_setopt(curl_handle, CURLOPT_URL, ourUrl.c_str());
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Awoo-Installer");
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, timeout);
        curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, timeout);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, writeDataBuffer);
        if (firstRange && secondRange) {
            const char * ourRange = (std::to_string(firstRange) + "-" + std::to_string(secondRange)).c_str();
            curl_easy_setopt(curl_handle, CURLOPT_RANGE, ourRange);
        }
        
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &stream);
        result = curl_easy_perform(curl_handle);
        
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();

        if (result == CURLE_OK) return stream.str();
        else {
            LOG_DEBUG(curl_easy_strerror(result));
            return "";
        }
    }
}