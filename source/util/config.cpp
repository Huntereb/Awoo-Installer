#include <filesystem>
#include <unistd.h>
#include <string.h>
#include "util/INIReader.h"
#include "util/config.hpp"

namespace inst::config {
    static const char* configBase = "[settings]\nignoreReqVers=%d\nvalidateNCAs=%d\noverClock=%d\ndeletePrompt=%d\nautoUpdate=%d\ngayMode=%d\nsigPatchesUrl=%s\ngAuthKey=%s\nlanguageSetting=%i\nusbAck=%d";
    std::string sigPatchesUrl;
    std::string gAuthKey;
    std::vector<std::string> updateInfo;
    int languageSetting;
    bool ignoreReqVers;
    bool validateNCAs;
    bool overClock;
    bool deletePrompt;
    bool autoUpdate;
    bool gayMode;
    bool usbAck;

    void parseConfig() {
        INIReader reader(configPath);
        ignoreReqVers = reader.GetBoolean("settings", "ignoreReqVers", true);
        validateNCAs = reader.GetBoolean("settings", "validateNCAs", true);
        overClock = reader.GetBoolean("settings", "overClock", false);
        deletePrompt = reader.GetBoolean("settings", "deletePrompt", true);
        autoUpdate = reader.GetBoolean("settings", "autoUpdate", true);
        gayMode = reader.GetBoolean("settings", "gayMode", false);
        sigPatchesUrl = reader.GetString("settings", "sigPatchesUrl", "https://github.com/Huntereb/Awoo-Installer/releases/download/SignaturePatches/patches.zip");
        gAuthKey = reader.GetString("settings", "gAuthKey", {0x41,0x49,0x7a,0x61,0x53,0x79,0x42,0x4d,0x71,0x76,0x34,0x64,0x58,0x6e,0x54,0x4a,0x4f,0x47,0x51,0x74,0x5a,0x5a,0x53,0x33,0x43,0x42,0x6a,0x76,0x66,0x37,0x34,0x38,0x51,0x76,0x78,0x53,0x7a,0x46,0x30});
        languageSetting = reader.GetInteger("settings", "languageSetting", 99);
        usbAck = reader.GetBoolean("settings", "usbAck", false);
        return;
    }

    void setConfig() {
        std::filesystem::remove(inst::config::configPath);
        char data[139 + sigPatchesUrl.size() + gAuthKey.size()];
        sprintf(data, configBase, ignoreReqVers, validateNCAs, overClock, deletePrompt, autoUpdate, gayMode, sigPatchesUrl.c_str(), gAuthKey.c_str(), languageSetting, usbAck);
        FILE * configFile = fopen(inst::config::configPath.c_str(), "w");
        fwrite(data, sizeof(char), strlen(data), configFile);
        fflush(configFile);
        fsync(fileno(configFile));
        fclose(configFile);
    }
}