#include <filesystem>
#include <vector>
#include <algorithm>
#include "switch.h"
#include "util.hpp"
#include "nx/ipc/tin_ipc.h"

namespace util {
    void initApp () {
        if (!std::filesystem::exists("sdmc:/switch")) std::filesystem::create_directory("sdmc:/switch");
        if (!std::filesystem::exists(appVariables::appDir)) std::filesystem::create_directory(appVariables::appDir);

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
}