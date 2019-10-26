#include "util/curl.hpp"
#include <curl/curl.h>
#include <string>

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
  size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

namespace inst::curl {
    bool downloadFile (const std::string ourUrl, const char *pagefilename) {
        CURL *curl_handle;
        CURLcode result;
        FILE *pagefile;
        
        curl_global_init(CURL_GLOBAL_ALL);
        
        /* init the curl session */ 
        curl_handle = curl_easy_init();
        
        /* set URL to get here */ 
        curl_easy_setopt(curl_handle, CURLOPT_URL, ourUrl.c_str());

        /* Misc variables */
        curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT_MS, 1000L);
        curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT_MS, 1000L);
        
        /* send all data to this function  */ 
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
        
        /* open the file */ 
        pagefile = fopen(pagefilename, "wb");
        
        /* write the page body to this file handle */ 
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, pagefile);
    
        /* get it! */ 
        result = curl_easy_perform(curl_handle);
        
        /* cleanup curl stuff */ 
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
    
        /* close the header file */ 
        fclose(pagefile);

        if (result == CURLE_OK) return true;
        else {
            printf(curl_easy_strerror(result));
            return false;
        }
    }
}