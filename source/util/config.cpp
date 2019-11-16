#include <filesystem>
#include <unistd.h>
#include <string.h>
#include "util/INIReader.h"
#include "util/config.hpp"

namespace inst::config {
    static const char* configBase = "[settings]\nignoreReqVers=%d\nvalidateNCAs=%d\ndeletePrompt=%d\ngayMode=%d\nsigPatchesUrl=%s\n";
    std::string sigPatchesUrl;
    bool ignoreReqVers;
    bool validateNCAs;
    bool deletePrompt;
    bool gayMode;

    void parseConfig() {
        INIReader reader(configPath);
        ignoreReqVers = reader.GetBoolean("settings", "ignoreReqVers", true);
        validateNCAs = reader.GetBoolean("settings", "validateNCAs", true);
        deletePrompt = reader.GetBoolean("settings", "deletePrompt", true);
        gayMode = reader.GetBoolean("settings", "gayMode", false);
        sigPatchesUrl = reader.GetString("settings", "sigPatchesUrl", "https://github.com/Huntereb/Awoo-Installer/releases/download/SignaturePatches/patches.zip");
        return;
    }

    void setConfig() {
        std::filesystem::remove(inst::config::configPath);
        char data[82 + sigPatchesUrl.size()];
        sprintf(data, configBase, ignoreReqVers, validateNCAs, deletePrompt, gayMode, sigPatchesUrl.c_str());
        FILE * configFile = fopen(inst::config::configPath.c_str(), "w");
        fwrite(data, sizeof(char), strlen(data), configFile);
        fflush(configFile);
        fsync(fileno(configFile));
        fclose(configFile);
    }
}