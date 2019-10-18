#include <filesystem>
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
}