#include <filesystem>
#include <vector>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <curl/curl.h>
#include <regex>
#include <arpa/inet.h>
#include <unistd.h>
#include "switch.h"
#include "util/util.hpp"
#include "nx/ipc/tin_ipc.h"
#include "util/INIReader.h"
#include "util/config.hpp"
#include "util/curl.hpp"
#include "ui/MainApplication.hpp"
#include "util/usb_comms_awoo.h"
#include "util/json.hpp"

namespace inst::util {
    void initApp () {
        // Seethe
        if (!pu::IsReiNX()) pu::IsAtmosphere();
        if (!std::filesystem::exists("sdmc:/switch")) std::filesystem::create_directory("sdmc:/switch");
        if (!std::filesystem::exists(inst::config::appDir)) std::filesystem::create_directory(inst::config::appDir);
        inst::config::parseConfig();
        if (!std::filesystem::exists(inst::config::configPath)) inst::config::setConfig();

        socketInitializeDefault();
        #ifdef __DEBUG__
            nxlinkStdio();
        #endif
        awoo_usbCommsInitialize();
    }

    void deinitApp () {
        socketExit();
        awoo_usbCommsExit();
    }

    void initInstallServices() {
        ncmInitialize();
        nsInitialize();
        nsextInitialize();
        esInitialize();
        splCryptoInitialize();
        splInitialize();
    }

    void deinitInstallServices() {
        ncmExit();
        nsExit();
        nsextExit();
        esExit();
        splCryptoExit();
        splExit();
    }

    struct caseInsensitiveLess : public std::binary_function< char,char,bool > {
        bool operator () (char x, char y) const {
            return toupper(static_cast< unsigned char >(x)) < toupper(static_cast< unsigned char >(y));
        }
    };

    bool ignoreCaseCompare(const std::string &a, const std::string &b) {
        return std::lexicographical_compare(a.begin(), a.end() , b.begin() ,b.end() , caseInsensitiveLess());
    }

    std::vector<std::filesystem::path> getDirectoryFiles(const std::string & dir, const std::vector<std::string> & extensions) {
        std::vector<std::filesystem::path> files;
        for(auto & p: std::filesystem::directory_iterator(dir))
        {
            if (std::filesystem::is_regular_file(p))
            {
                std::string ourExtension = p.path().extension().string();
                std::transform(ourExtension.begin(), ourExtension.end(), ourExtension.begin(), ::tolower);
                if (extensions.empty() || std::find(extensions.begin(), extensions.end(), ourExtension) != extensions.end())
                {
                    files.push_back(p.path());
                }
            }
        }
        std::sort(files.begin(), files.end(), ignoreCaseCompare);
        return files;
    }

    std::vector<std::filesystem::path> getDirsAtPath(const std::string & dir) {
        std::vector<std::filesystem::path> files;
        for(auto & p: std::filesystem::directory_iterator(dir))
        {
            if (std::filesystem::is_directory(p))
            {
                    files.push_back(p.path());
            }
        }
        std::sort(files.begin(), files.end(), ignoreCaseCompare);
        return files;
    }

    bool removeDirectory(std::string dir) {
        try {
            for(auto & p: std::filesystem::recursive_directory_iterator(dir))
            {
                if (std::filesystem::is_regular_file(p))
                {
                    std::filesystem::remove(p);
                }
            }
            rmdir(dir.c_str());
            return true;
        }
        catch (std::filesystem::filesystem_error & e) {
            return false;
        }
    }

    bool copyFile(std::string inFile, std::string outFile) {
       char ch;
       std::ifstream f1(inFile);
       std::ofstream f2(outFile);

       if(!f1 || !f2) return false;
       
       while(f1 && f1.get(ch)) f2.put(ch);
       return true;
    }

    std::string formatUrlString(std::string ourString) {
        std::stringstream ourStream(ourString);
        std::string segment;
        std::vector<std::string> seglist;

        while(std::getline(ourStream, segment, '/')) {
            seglist.push_back(segment);
        }

        CURL *curl = curl_easy_init();
        int outlength;
        std::string finalString = curl_easy_unescape(curl, seglist[seglist.size() - 1].c_str(), seglist[seglist.size() - 1].length(), &outlength);
        curl_easy_cleanup(curl);

        return finalString;
    }

    std::string shortenString(std::string ourString, int ourLength, bool isFile) {
        std::filesystem::path ourStringAsAPath = ourString;
        std::string ourExtension = ourStringAsAPath.extension().string();
        if (ourString.size() - ourExtension.size() > (unsigned long)ourLength) {
            if(isFile) return (std::string)ourString.substr(0,ourLength) + "(...)" + ourExtension;
            else return (std::string)ourString.substr(0,ourLength) + "...";
        } else return ourString;
    }

    std::string readTextFromFile(std::string ourFile) {
        if (std::filesystem::exists(ourFile)) {
            FILE * file = fopen(ourFile.c_str(), "r");
            char line[1024];
            fgets(line, 1024, file);
            std::string url = line;
            fflush(file);
            fclose(file);
            return url;
        }
        return "";
    }

    std::string softwareKeyboard(std::string guideText, std::string initialText, int LenMax) {
        Result rc=0;
        SwkbdConfig kbd;
        char tmpoutstr[LenMax + 1] = {0};
        rc = swkbdCreate(&kbd, 0);
        if (R_SUCCEEDED(rc)) {
            swkbdConfigMakePresetDefault(&kbd);
            swkbdConfigSetGuideText(&kbd, guideText.c_str());
            swkbdConfigSetInitialText(&kbd, initialText.c_str());
            swkbdConfigSetStringLenMax(&kbd, LenMax);
            rc = swkbdShow(&kbd, tmpoutstr, sizeof(tmpoutstr));
            swkbdClose(&kbd);
            if (R_SUCCEEDED(rc) && tmpoutstr[0] != 0) return(((std::string)(tmpoutstr)));
        }
        return "";
    }

    std::string getDriveFileName(std::string fileId) {
        std::string htmlData = inst::curl::downloadToBuffer("https://drive.google.com/file/d/" + fileId  + "/view");
        if (htmlData.size() > 0) {
            std::smatch ourMatches;
            std::regex ourRegex("<title>\\s*(.+?)\\s*</title>");
            std::regex_search(htmlData, ourMatches, ourRegex);
            if (ourMatches.size() > 1) {
                if (ourMatches[1].str() == "Google Drive -- Page Not Found") return "";
                return ourMatches[1].str().substr(0, ourMatches[1].str().size() - 15);
             }
        }
        return "";
    }

    std::vector<uint32_t> setClockSpeed(int deviceToClock, uint32_t clockSpeed) {
        uint32_t hz = 0;
        uint32_t previousHz = 0;

        if (deviceToClock > 2 || deviceToClock < 0) return {0,0};

        if(hosversionAtLeast(8,0,0)) {
            ClkrstSession session = {0};
            PcvModuleId pcvModuleId;
            pcvInitialize();
            clkrstInitialize();

            switch (deviceToClock) {
                case 0:
                    pcvGetModuleId(&pcvModuleId, PcvModule_CpuBus);
                    break;
                case 1:
                    pcvGetModuleId(&pcvModuleId, PcvModule_GPU);
                    break;
                case 2:
                    pcvGetModuleId(&pcvModuleId, PcvModule_EMC);
                    break;
            }

            clkrstOpenSession(&session, pcvModuleId, 3);
            clkrstGetClockRate(&session, &previousHz);
            clkrstSetClockRate(&session, clockSpeed);
            clkrstGetClockRate(&session, &hz);

            pcvExit();
            clkrstCloseSession(&session);
            clkrstExit();

            return {previousHz, hz};
        } else {
            PcvModule pcvModule;
            pcvInitialize();

            switch (deviceToClock) {
                case 0:
                    pcvModule = PcvModule_CpuBus;
                    break;
                case 1:
                    pcvModule = PcvModule_GPU;
                    break;
                case 2:
                    pcvModule = PcvModule_EMC;
                    break;
            }

            pcvGetClockRate(pcvModule, &previousHz);
            pcvSetClockRate(pcvModule, clockSpeed);
            pcvGetClockRate(pcvModule, &hz);
            
            pcvExit();

            return {previousHz, hz};
        }
    }

    std::string getIPAddress() {
        struct in_addr addr = {(in_addr_t) gethostid()};
        return inet_ntoa(addr);
    }
    
    int getUsbState() {
        u32 usbState = 0;
        usbDsGetState(&usbState);
        return usbState;
    }

    void playAudio(std::string audioPath) {
        int audio_rate = 22050;
        Uint16 audio_format = AUDIO_S16SYS;
        int audio_channels = 2;
        int audio_buffers = 4096;

        if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) return;

        Mix_Chunk *sound = NULL;
        sound = Mix_LoadWAV(audioPath.c_str());
        if(sound == NULL) {
            Mix_FreeChunk(sound);
            Mix_CloseAudio();
            return;
        }

        int channel = Mix_PlayChannel(-1, sound, 0);
        if(channel == -1) {
            Mix_FreeChunk(sound);
            Mix_CloseAudio();
            return;
        }

        while(Mix_Playing(channel) != 0);

        Mix_FreeChunk(sound);
        Mix_CloseAudio();

        return;
    }
    
   std::vector<std::string> checkForAppUpdate () {
        try {
            std::string jsonData = inst::curl::downloadToBuffer("https://api.github.com/repos/Huntereb/Awoo-Installer/releases/latest", 0, 0, 1000L);
            if (jsonData.size() == 0) return {};
            nlohmann::json ourJson = nlohmann::json::parse(jsonData);
            if (ourJson["tag_name"].get<std::string>() != inst::config::appVersion) return {ourJson["tag_name"].get<std::string>(), ourJson["assets"][0]["browser_download_url"].get<std::string>()};
        } catch (...) {}
        return {};
    }
}