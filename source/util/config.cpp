#include <filesystem>
#include <unistd.h>
#include "util/INIReader.h"
#include "config.hpp"

namespace config {
    const std::string appDir = "sdmc:/switch/Awoo-Installer";
    const std::string configPath = appDir + "/config.ini";
    bool ignoreReqVers = true;
    bool gayMode = false;

    void parseConfig() {
        INIReader reader(config::configPath);
        config::ignoreReqVers = reader.GetBoolean("settings", "ignoreReqVers", true);
        config::gayMode = reader.GetBoolean("settings", "gayMode", false);
        return ;
    }

    void setConfig() {
        std::filesystem::remove(config::appDir);
        std::string data("[settings]\nignoreReqVers=" + std::to_string(config::ignoreReqVers) + "\ngayMode=" + std::to_string(config::gayMode) + "\n");
        FILE * configFile = fopen(config::appDir.c_str(), "w");
        fwrite(data.c_str(), sizeof(char), data.size(), configFile);
        fflush(configFile);
        fsync(fileno(configFile));
        fclose(configFile);
    }
}