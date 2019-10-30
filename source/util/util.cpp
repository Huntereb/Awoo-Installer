#include <filesystem>
#include <vector>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <curl/curl.h>
#include "switch.h"
#include "util/util.hpp"
#include "nx/ipc/tin_ipc.h"
#include "util/INIReader.h"
#include "util/config.hpp"

namespace inst::util {
    void initApp () {
        if (!std::filesystem::exists("sdmc:/switch")) std::filesystem::create_directory("sdmc:/switch");
        if (!std::filesystem::exists(inst::config::appDir)) std::filesystem::create_directory(inst::config::appDir);
        if (std::filesystem::exists(inst::config::configPath)) inst::config::parseConfig();
        else inst::config::setConfig();

        socketInitializeDefault();
        #ifdef __DEBUG__
            nxlinkStdio();
        #endif
        plInitialize();
        setInitialize();
        ncmInitialize();
        nsInitialize();
        nsextInitialize();
        esInitialize();
    }

    void deinitApp () {
        socketExit();
        plExit();
        setExit();
        ncmExit();
        nsExit();
        nsextExit();
        esExit();
    }

    std::vector<std::filesystem::path> getDirectoryFiles(const std::string & dir, const std::vector<std::string> & extensions) {
        std::vector<std::filesystem::path> files;
        for(auto & p: std::filesystem::directory_iterator(dir))
        {
            if (std::filesystem::is_regular_file(p))
            {
                    if (extensions.empty() || std::find(extensions.begin(), extensions.end(), p.path().extension().string()) != extensions.end())
                {
                    files.push_back(p.path());
                }
            }
        }
        std::sort(files.begin(), files.end());
        std::reverse(files.begin(), files.end());
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
        if (ourString.size() > (unsigned long)ourLength) {
            if(isFile) return (std::string)ourString.substr(0,ourLength) + "(...)" + ourStringAsAPath.extension().string();
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
}