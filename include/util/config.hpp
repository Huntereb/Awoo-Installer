#pragma once

namespace inst::config {
    extern const std::string appDir;
    extern const std::string configPath;
    extern const std::string appVersion;
    extern const std::string gAuthKey;
    extern std::string sigPatchesUrl;
    extern bool ignoreReqVers;
    extern bool deletePrompt;
    extern bool gayMode;

    void parseConfig();
    void setConfig();
}