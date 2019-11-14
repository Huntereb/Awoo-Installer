#include <filesystem>
#include <unistd.h>
#include "util/INIReader.h"
#include "util/config.hpp"

namespace inst::config {
    const std::string appDir = "sdmc:/switch/Awoo-Installer";
    const std::string configPath = appDir + "/config.ini";
    const std::string gAuthKey = {0x41,0x49,0x7a,0x61,0x53,0x79,0x42,0x4d,0x71,0x76,0x34,0x64,0x58,0x6e,0x54,0x4a,0x4f,0x47,0x51,0x74,0x5a,0x5a,0x53,0x33,0x43,0x42,0x6a,0x76,0x66,0x37,0x34,0x38,0x51,0x76,0x78,0x53,0x7a,0x46,0x30};
    std::string sigPatchesUrl = "https://github.com/Huntereb/Awoo-Installer/releases/download/SignaturePatches/patches.zip";
    bool ignoreReqVers = true;
    bool deletePrompt = true;
    bool gayMode = false;

    void parseConfig() {
        INIReader reader(inst::config::configPath);
        inst::config::ignoreReqVers = reader.GetBoolean("settings", "ignoreReqVers", inst::config::ignoreReqVers);
        inst::config::deletePrompt = reader.GetBoolean("settings", "deletePrompt", inst::config::deletePrompt);
        inst::config::gayMode = reader.GetBoolean("settings", "gayMode", inst::config::gayMode);
        inst::config::sigPatchesUrl = reader.GetString("settings", "sigPatchesUrl", inst::config::sigPatchesUrl);
        return;
    }

    void setConfig() {
        std::filesystem::remove(inst::config::configPath);
        std::string data("[settings]\nignoreReqVers=" + std::to_string(inst::config::ignoreReqVers) + "\ndeletePrompt=" + std::to_string(inst::config::deletePrompt) + "\ngayMode=" + std::to_string(inst::config::gayMode) + "\nsigPatchesUrl=" + inst::config::sigPatchesUrl + "\n");
        FILE * configFile = fopen(inst::config::configPath.c_str(), "w");
        fwrite(data.c_str(), sizeof(char), data.size(), configFile);
        fflush(configFile);
        fsync(fileno(configFile));
        fclose(configFile);
    }
}