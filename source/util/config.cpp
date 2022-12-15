#include <fstream>
#include <iomanip>
#include "util/config.hpp"
#include "util/json.hpp"

namespace inst::config {
    std::string gAuthKey;
    std::string sigPatchesUrl;
    std::string lastNetUrl;
    std::vector<std::string> updateInfo;
    int languageSetting;
    bool autoUpdate;
    bool deletePrompt;
    bool gayMode;
    bool ignoreReqVers;
    bool overClock;
    bool usbAck;
    bool validateNCAs;

    void setConfig() {
        nlohmann::json j = {
            {"autoUpdate", autoUpdate},
            {"deletePrompt", deletePrompt},
            {"gAuthKey", gAuthKey},
            {"gayMode", gayMode},
            {"ignoreReqVers", ignoreReqVers},
            {"languageSetting", languageSetting},
            {"overClock", overClock},
            {"sigPatchesUrl", sigPatchesUrl},
            {"usbAck", usbAck},
            {"validateNCAs", validateNCAs},
            {"lastNetUrl", lastNetUrl}
        };
        std::ofstream file(inst::config::configPath);
        file << std::setw(4) << j << std::endl;
    }

    void parseConfig() {
        try {
            std::ifstream file(inst::config::configPath);
            nlohmann::json j;
            file >> j;
            autoUpdate = j["autoUpdate"].get<bool>();
            deletePrompt = j["deletePrompt"].get<bool>();
            gAuthKey = j["gAuthKey"].get<std::string>();
            gayMode = j["gayMode"].get<bool>();
            ignoreReqVers = j["ignoreReqVers"].get<bool>();
            languageSetting = j["languageSetting"].get<int>();
            overClock = j["overClock"].get<bool>();
            sigPatchesUrl = j["sigPatchesUrl"].get<std::string>();
            usbAck = j["usbAck"].get<bool>();
            validateNCAs = j["validateNCAs"].get<bool>();
            lastNetUrl = j["lastNetUrl"].get<std::string>();
        }
        catch (...) {
            // If loading values from the config fails, we just load the defaults and overwrite the old config
            gAuthKey = {0x41,0x49,0x7a,0x61,0x53,0x79,0x42,0x4d,0x71,0x76,0x34,0x64,0x58,0x6e,0x54,0x4a,0x4f,0x47,0x51,0x74,0x5a,0x5a,0x53,0x33,0x43,0x42,0x6a,0x76,0x66,0x37,0x34,0x38,0x51,0x76,0x78,0x53,0x7a,0x46,0x30};
            sigPatchesUrl = "https://sigmapatches.coomer.party/sigpatches.zip";
            languageSetting = 99;
            autoUpdate = true;
            deletePrompt = true;
            gayMode = false;
            ignoreReqVers = true;
            overClock = false;
            usbAck = false;
            validateNCAs = true;
            lastNetUrl = "https://";
            setConfig();
        }
        if (sigPatchesUrl == "https://github.com/Huntereb/Awoo-Installer/releases/download/SignaturePatches/patches.zip")
            sigPatchesUrl = "https://sigmapatches.coomer.party/sigpatches.zip";
    }
}