#include <filesystem>
#include <unistd.h>
#include <string.h>
#include "util/INIReader.h"
#include "util/config.hpp"

namespace inst::config {
    static const char* configBase = "[settings]\nignoreReqVers=%d\nvalidateNCAs=%d\noverClock=%d\ndeletePrompt=%d\ngayMode=%d\nsigPatchesUrl=%s\nusbAck=%d";
    std::string sigPatchesUrl;
    bool ignoreReqVers;
    bool validateNCAs;
    bool overClock;
    bool deletePrompt;
    bool gayMode;
    bool usbAck;

    void parseConfig() {
        INIReader reader(configPath);
        ignoreReqVers = reader.GetBoolean("settings", "ignoreReqVers", true);
        validateNCAs = reader.GetBoolean("settings", "validateNCAs", true);
        overClock = reader.GetBoolean("settings", "overClock", false);
        deletePrompt = reader.GetBoolean("settings", "deletePrompt", true);
        gayMode = reader.GetBoolean("settings", "gayMode", false);
        sigPatchesUrl = reader.GetString("settings", "sigPatchesUrl", "https://github.com/Huntereb/Awoo-Installer/releases/download/SignaturePatches/patches.zip");
        usbAck = reader.GetBoolean("settings", "usbAck", false);
        return;
    }

    void setConfig() {
        std::filesystem::remove(inst::config::configPath);
        char data[96 + sigPatchesUrl.size()];
        sprintf(data, configBase, ignoreReqVers, validateNCAs, overClock, deletePrompt, gayMode, sigPatchesUrl.c_str(), usbAck);
        FILE * configFile = fopen(inst::config::configPath.c_str(), "w");
        fwrite(data, sizeof(char), strlen(data), configFile);
        fflush(configFile);
        fsync(fileno(configFile));
        fclose(configFile);
    }
}