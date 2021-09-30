/*
Copyright (c) 2017-2018 Adubbz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "util/network_util.hpp"

#include <switch.h>
#include <curl/curl.h>
#include <algorithm>
#include <cstring>
#include <sstream>
#include "util/error.hpp"
#include "ui/MainApplication.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;
}

namespace tin::network
{
    // HTTPHeader

    HTTPHeader::HTTPHeader(std::string url) :
        m_url(url)
    {
    }

    size_t HTTPHeader::ParseHTMLHeader(char* bytes, size_t size, size_t numItems, void* userData)
    {
        HTTPHeader* header = reinterpret_cast<HTTPHeader*>(userData);
        size_t numBytes = size * numItems;
        std::string line(bytes, numBytes);

        // Remove any newlines or carriage returns
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        // Split into key and value
        if (!line.empty())
        {
            auto keyEnd = line.find(": ");

            if (keyEnd != 0)
            {
                std::string key = line.substr(0, keyEnd);
                std::string value = line.substr(keyEnd + 2);

                // Make key lowercase
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                header->m_values[key] = value;
            }
        }

        return numBytes;
    }

    void HTTPHeader::PerformRequest()
    {
        // We don't want any existing values to get mixed up with this request
        m_values.clear();

        CURL* curl = curl_easy_init();
        CURLcode rc = (CURLcode)0;

        if (!curl)
        {
            THROW_FORMAT("Failed to initialize curl\n");
        }

        curl_easy_setopt(curl, CURLOPT_URL, m_url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, true);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "tinfoil");
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &tin::network::HTTPHeader::ParseHTMLHeader);

        rc = curl_easy_perform(curl);
        if (rc != CURLE_OK)
        {
            THROW_FORMAT("Failed to retrieve HTTP Header: %s\n", curl_easy_strerror(rc));
        }

        u64 httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);

        if (httpCode != 200 && httpCode != 204)
        {
            THROW_FORMAT("Unexpected HTTP response code when retrieving header: %lu\n", httpCode);
        }
    }

    bool HTTPHeader::HasValue(std::string key)
    {
        return m_values.count(key);
    }

    std::string HTTPHeader::GetValue(std::string key)
    {
        return m_values[key];
    }

    // End HTTPHeader
    // HTTPDownload

    HTTPDownload::HTTPDownload(std::string url) :
        m_url(url), m_header(url)
    {
        // The header won't be populated until we do this
        m_header.PerformRequest();

        if (m_header.HasValue("accept-ranges"))
        {
            m_rangesSupported = m_header.GetValue("accept-ranges") == "bytes";
        }
        else
        {
            CURL* curl = curl_easy_init();
            CURLcode rc = (CURLcode)0;

            if (!curl)
            {
                THROW_FORMAT("Failed to initialize curl\n");
            }

            curl_easy_setopt(curl, CURLOPT_URL, m_url.c_str());
            curl_easy_setopt(curl, CURLOPT_NOBODY, true);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "tinfoil");
            curl_easy_setopt(curl, CURLOPT_RANGE, "0-0");

            rc = curl_easy_perform(curl);
            if (rc != CURLE_OK)
            {
                THROW_FORMAT("Failed to retrieve HTTP Header: %s\n", curl_easy_strerror(rc));
            }

            u64 httpCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
            curl_easy_cleanup(curl);

            m_rangesSupported = httpCode == 206;
        }
    }

    size_t HTTPDownload::ParseHTMLData(char* bytes, size_t size, size_t numItems, void* userData)
    {
        auto streamFunc = *reinterpret_cast<std::function<size_t (u8* bytes, size_t size)>*>(userData);
        size_t numBytes = size * numItems;

        if (streamFunc != nullptr)
            return streamFunc((u8*)bytes, numBytes);

        return numBytes;
    }

    void HTTPDownload::BufferDataRange(void* buffer, size_t offset, size_t size, std::function<void (size_t sizeRead)> progressFunc)
    {
        size_t sizeRead = 0;

        auto streamFunc = [&](u8* streamBuf, size_t streamBufSize) -> size_t
        {
            if (sizeRead + streamBufSize > size)
            {
                LOG_DEBUG("New read size 0x%lx would exceed total expected size 0x%lx\n", sizeRead + streamBufSize, size);
                return 0;
            }

            if (progressFunc != nullptr)
                progressFunc(sizeRead);

            memcpy(reinterpret_cast<u8*>(buffer) + sizeRead, streamBuf, streamBufSize);
            sizeRead += streamBufSize;
            return streamBufSize;
        };

        this->StreamDataRange(offset, size, streamFunc);
    }

    int HTTPDownload::StreamDataRange(size_t offset, size_t size, std::function<size_t (u8* bytes, size_t size)> streamFunc)
    {
        if (!m_rangesSupported)
        {
            THROW_FORMAT("Attempted range request when ranges aren't supported!\n");
        }

        auto writeDataFunc = streamFunc;

        CURL* curl = curl_easy_init();
        CURLcode rc = (CURLcode)0;

        if (!curl)
        {
            THROW_FORMAT("Failed to initialize curl\n");
        }

        std::stringstream ss;
        ss << offset << "-" << (offset + size - 1);
        auto range = ss.str();

        curl_easy_setopt(curl, CURLOPT_URL, m_url.c_str());
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "tinfoil");
        curl_easy_setopt(curl, CURLOPT_RANGE, range.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeDataFunc);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &tin::network::HTTPDownload::ParseHTMLData);

        rc = curl_easy_perform(curl);

        u64 httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);

        if (httpCode != 206 || rc != CURLE_OK) return 1;
        return 0;
    }

    // End HTTPDownload

    size_t WaitReceiveNetworkData(int sockfd, void* buf, size_t len)
    {
        int ret = 0;
        size_t read = 0;

        while ((((ret = recv(sockfd, (u8*)buf + read, len - read, 0)) > 0 && (read += ret) < len) || errno == EAGAIN)) 
        {
            errno = 0;
        }

        return read;
    }

    size_t WaitSendNetworkData(int sockfd, void* buf, size_t len)
    {
        int ret = 0;
        size_t written = 0;

        while (written < len)
        {            
            inst::ui::mainApp->UpdateButtons();
            u64 kDown = inst::ui::mainApp->GetButtonsDown();
            if (kDown & HidNpadButton_B)  // Break if user clicks 'B'
                break;

            errno = 0;
            ret = send(sockfd, (u8*)buf + written, len - written, 0);
            
            if (ret < 0){ // If error
                if (errno == EWOULDBLOCK || errno == EAGAIN){ // Is it because other side is busy?
                    sleep(5);
                    continue;
                }
                break; // No? Die.
            }
            
            written += ret;
        }
    
        return written;
    }

    void NSULDrop(std::string url)
    {
        CURL* curl = curl_easy_init();

        if (!curl)
        {
            THROW_FORMAT("Failed to initialize curl\n");
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DROP");
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "tinfoil");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 50); 

        curl_easy_perform(curl); // ignore returning value

        curl_easy_cleanup(curl);
    }
}