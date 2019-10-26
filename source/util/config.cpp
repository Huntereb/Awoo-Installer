#include <filesystem>
#include <unistd.h>
#include "util/INIReader.h"
#include "config.hpp"

namespace inst::config {
    const std::string appDir = "sdmc:/switch/Awoo-Installer";
    const std::string configPath = appDir + "/config.ini";
    bool ignoreReqVers = true;
    bool gayMode = false;

    void parseConfig() {
        INIReader reader(inst::config::configPath);
        inst::config::ignoreReqVers = reader.GetBoolean("settings", "ignoreReqVers", true);
        inst::config::gayMode = reader.GetBoolean("settings", "gayMode", false);
        return;
    }

    void setConfig() {
        std::filesystem::remove(inst::config::configPath);
        std::string data("[settings]\nignoreReqVers=" + std::to_string(inst::config::ignoreReqVers) + "\ngayMode=" + std::to_string(inst::config::gayMode) + "\n");
        FILE * configFile = fopen(inst::config::configPath.c_str(), "w");
        fwrite(data.c_str(), sizeof(char), data.size(), configFile);
        fflush(configFile);
        fsync(fileno(configFile));
        fclose(configFile);
    }
}