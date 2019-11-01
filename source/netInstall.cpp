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

#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <curl/curl.h>

#include <switch.h>
#include "util/network_util.hpp"
#include "install/install_nsp_remote.hpp"
#include "install/http_nsp.hpp"
#include "install/install.hpp"
#include "util/error.hpp"

#include "ui/MainApplication.hpp"
#include "netInstall.hpp"
#include "nspInstall.hpp"
#include "util/config.hpp"
#include "util/util.hpp"

const unsigned int MAX_URL_SIZE = 1024;
const unsigned int MAX_URLS = 256;
const int REMOTE_INSTALL_PORT = 2000;
static int m_serverSocket = 0;
static int m_clientSocket = 0;

namespace inst::ui {
    extern MainApplication *mainApp;

    void setNetInfoText(std::string ourText){
        mainApp->netinstPage->pageInfoText->SetText(ourText);
        mainApp->CallForRender();
    }
}

namespace netInstStuff{
    FsStorageId m_destStorageId = FsStorageId_SdCard;

    void InitializeServerSocket() try
    {
        // Create a socket
        m_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

        if (m_serverSocket < -1)
        {
            THROW_FORMAT("Failed to create a server socket. Error code: %u\n", errno);
        }

        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(REMOTE_INSTALL_PORT);
        server.sin_addr.s_addr = htonl(INADDR_ANY);

        if (bind(m_serverSocket, (struct sockaddr*) &server, sizeof(server)) < 0)
        {
            THROW_FORMAT("Failed to bind server socket. Error code: %u\n", errno);
        }

        // Set as non-blocking
        fcntl(m_serverSocket, F_SETFL, fcntl(m_serverSocket, F_GETFL, 0) | O_NONBLOCK);

        if (listen(m_serverSocket, 5) < 0) 
        {
            THROW_FORMAT("Failed to listen on server socket. Error code: %u\n", errno);
        }
    }
    catch (std::exception& e)
    {
        printf("Failed to initialize server socket!\n");
        fprintf(stdout, "%s", e.what());

        if (m_serverSocket != 0)
        {
            close(m_serverSocket);
            m_serverSocket = 0;
        }
        inst::ui::mainApp->CreateShowDialog("Failed to initialize server socket!", (std::string)e.what(), {"OK"}, true);
    }

    void OnUnwound()
    {
        printf("unwinding view\n");
        if (m_clientSocket != 0)
        {
            close(m_clientSocket);
            m_clientSocket = 0;
        }

        curl_global_cleanup();
    }

    void installNspLan(std::string ourUrl, int ourStorage)
    {
        appletLockExit();
        inst::ui::loadInstallScreen();
        if (ourStorage) m_destStorageId = FsStorageId_NandUser;
        try {
            inst::ui::setTopInstInfoText("Installing " + inst::util::formatUrlString(ourUrl) + "...");

            tin::install::nsp::HTTPNSP httpNSP(ourUrl);

            printf("%s %s\n", "NSP Install request from", ourUrl.c_str());
            tin::install::nsp::RemoteNSPInstall install(m_destStorageId, inst::config::ignoreReqVers, &httpNSP);

            printf("%s\n", "Preparing installation");
            inst::ui::setInstInfoText("Preparing installation...");
            install.Prepare();

            install.Begin();

            printf("%s\n", "Telling the server we're done installing");
            inst::ui::setInstInfoText("Telling the server we're done installing...");
            // Send 1 byte ack to close the server
            u8 ack = 0;
            tin::network::WaitSendNetworkData(m_clientSocket, &ack, sizeof(u8));
            inst::ui::mainApp->CreateShowDialog(inst::util::shortenString(inst::util::formatUrlString(ourUrl), 64, true) + " installed!", "", {"OK"}, true);
        }
        catch (std::exception& e) {
            printf("Failed to install NSP");
            printf("%s", e.what());
            fprintf(stdout, "%s", e.what());
            inst::ui::mainApp->CreateShowDialog("Failed to install NSP!", "Partially installed NSP contents can be removed from the System Settings applet.\n\n" + (std::string)e.what(), {"OK"}, true);
        }
        
        printf("Done");
        appletUnlockExit();
        inst::ui::loadMainMenu();
        return;
    }

    std::vector<std::string> OnSelected()
    {
        OnUnwound();
        try
        {
            ASSERT_OK(curl_global_init(CURL_GLOBAL_ALL), "Curl failed to initialized");

            // Initialize the server socket if it hasn't already been
            if (m_serverSocket == 0)
            {
                InitializeServerSocket();

                if (m_serverSocket <= 0)
                {
                    THROW_FORMAT("Server socket failed to initialize.\n");
                }
            }

            struct in_addr addr = {(in_addr_t) gethostid()};
            std::string ourIPAddr(inet_ntoa(addr));
            // If our IP is 127.0.0.1, cancel because we aren't connected to a network
            if (ourIPAddr == "1.0.0.127") {
                inst::ui::mainApp->CreateShowDialog("Network connection not available", "Check that airplane mode is disabled and you're connected to a local network.", {"OK"}, true);
                return {};
            }
            inst::ui::setNetInfoText("Waiting for a connection... Your Switch's IP Address is: " + ourIPAddr);

            printf("%s %s\n", "Switch IP is ", inet_ntoa(addr));
            printf("%s\n", "Waiting for network");
            printf("%s\n", "B to cancel");
            
            std::vector<std::string> urls;

            while (true)
            {
                // Break on input pressed
                hidScanInput();
                u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

                if (kDown & KEY_B)
                {
                    break;
                }
                if (kDown & KEY_Y)
                {
                    return {"supplyUrl"};
                }

                struct sockaddr_in client;
                socklen_t clientLen = sizeof(client);

                m_clientSocket = accept(m_serverSocket, (struct sockaddr*)&client, &clientLen);

                if (m_clientSocket >= 0)
                {
                    printf("%s\n", "Server accepted");
                    u32 size = 0;
                    tin::network::WaitReceiveNetworkData(m_clientSocket, &size, sizeof(u32));
                    size = ntohl(size);

                    printf("Received url buf size: 0x%x\n", size);

                    if (size > MAX_URL_SIZE * MAX_URLS)
                    {
                        THROW_FORMAT("URL size %x is too large!\n", size);
                    }

                    // Make sure the last string is null terminated
                    auto urlBuf = std::make_unique<char[]>(size+1);
                    memset(urlBuf.get(), 0, size+1);

                    tin::network::WaitReceiveNetworkData(m_clientSocket, urlBuf.get(), size);

                    // Split the string up into individual URLs
                    std::stringstream urlStream(urlBuf.get());
                    std::string segment;
                    std::string nspExt = ".nsp";

                    while (std::getline(urlStream, segment, '\n'))
                    {
                        if (segment.compare(segment.size() - nspExt.size(), nspExt.size(), nspExt) == 0)
                            urls.push_back(segment);
                    }

                    break;
                }
                else if (errno != EAGAIN)
                {
                    THROW_FORMAT("Failed to open client socket with code %u\n", errno);
                }
            }

            return urls;

        }
        catch (std::runtime_error& e)
        {
            printf("Failed to perform remote install!\n");
            printf("%s", e.what());
            fprintf(stdout, "%s", e.what());
            inst::ui::mainApp->CreateShowDialog("Failed to perform remote install!", (std::string)e.what(), {"OK"}, true);
            return {};
        }
    }
}
