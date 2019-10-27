#pragma once
#include <string>

namespace inst::config {
    extern const std::string appDir;
    extern const std::string configPath;
    extern std::string sigPatchesUrl;
    extern bool ignoreReqVers;
    extern bool gayMode;

    void parseConfig();
    void setConfig();
}