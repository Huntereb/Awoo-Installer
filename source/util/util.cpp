#include <filesystem>
#include <vector>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include "switch.h"
#include "util/util.hpp"
#include "nx/ipc/tin_ipc.h"
#include "util/INIReader.h"
#include "config.hpp"

namespace util {
    void initApp () {
        if (!std::filesystem::exists("sdmc:/switch")) std::filesystem::create_directory("sdmc:/switch");
        if (!std::filesystem::exists(config::appDir)) std::filesystem::create_directory(config::appDir);
        config::parseConfig();

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
}