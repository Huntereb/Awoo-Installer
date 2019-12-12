#pragma once

namespace inst::config {
    static const std::string appDir = "sdmc:/switch/Awoo-Installer";
    static const std::string configPath = appDir + "/config.ini";
    static const std::string appVersion = "1.2.2";

    extern std::string sigPatchesUrl;
    extern std::string gAuthKey;
    extern bool ignoreReqVers;
    extern bool validateNCAs;
    extern bool overClock;
    extern bool deletePrompt;
    extern bool gayMode;
    extern bool usbAck;

    void parseConfig();
    void setConfig();
}