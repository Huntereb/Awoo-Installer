#include <switch.h>
#include "util/error.hpp"
#include "ui/MainApplication.hpp"
#include "util/curl.hpp"
#include "util/util.hpp"
#include "util/unzip.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;
}

namespace sig {
    void installSigPatches () {
        bpcInitialize();
        try {
            std::string patchesVersion = inst::util::readTextFromFile("sdmc:/atmosphere/exefs_patches/es_patches/patches.txt");
            std::string versionText = "";
            std::string installButtonText = "sig.install"_lang;
            if (patchesVersion != "") {
                versionText = "\n\n" + "sig.version_text"_lang + patchesVersion + ".";
                installButtonText = "sig.update"_lang;
            }
            int ourResult = inst::ui::mainApp->CreateShowDialog("sig.title0"_lang, "sig.desc0"_lang + versionText, {installButtonText, "sig.uninstall"_lang, "common.cancel"_lang}, true);
            if (ourResult == 0) {
                if (inst::util::getIPAddress() == "1.0.0.127") {
                    inst::ui::mainApp->CreateShowDialog("main.net.title"_lang, "main.net.desc"_lang, {"common.ok"_lang}, true);
                    return;
                }
                if (!inst::util::copyFile("sdmc:/bootloader/patches.ini", inst::config::appDir + "/patches.ini.old")) {
                    if (inst::ui::mainApp->CreateShowDialog("sig.backup_failed"_lang, "sig.backup_failed_desc"_lang, {"common.yes"_lang, "common.no"_lang}, false)) return;
                }
                std::string ourPath = inst::config::appDir + "/patches.zip";
                bool didDownload = inst::curl::downloadFile(inst::config::sigPatchesUrl, ourPath.c_str());
                bool didExtract = false;
                if (didDownload) didExtract = inst::zip::extractFile(ourPath, "sdmc:/");
                else {
                    inst::ui::mainApp->CreateShowDialog("sig.download_failed"_lang, "sig.download_failed_desc"_lang, {"common.ok"_lang}, true);
                    return;
                }
                std::filesystem::remove(ourPath);
                if (didExtract) {
                    patchesVersion = inst::util::readTextFromFile("sdmc:/atmosphere/exefs_patches/es_patches/patches.txt");
                    versionText = "";
                    if (patchesVersion != "") versionText = "sig.version_text2"_lang + patchesVersion + "! ";
                    if (inst::ui::mainApp->CreateShowDialog("sig.install_complete"_lang, versionText + "\n\n" + "sig.complete_desc"_lang, {"sig.restart"_lang, "sig.later"_lang}, false) == 0) bpcRebootSystem();
                }
                else {
                    inst::ui::mainApp->CreateShowDialog("sig.extract_failed"_lang, "", {"common.ok"_lang}, true);
                    return;
                }
                return;
            } else if (ourResult == 1) {
                if (!inst::util::copyFile( inst::config::appDir + "/patches.ini.old", "sdmc:/bootloader/patches.ini")) {
                    if (inst::ui::mainApp->CreateShowDialog("sig.restore_failed"_lang, "", {"common.yes"_lang, "common.no"_lang}, false)) return;
                } else std::filesystem::remove(inst::config::appDir + "/patches.ini.old");
                if (inst::util::removeDirectory("sdmc:/atmosphere/exefs_patches/es_patches")) {
                    if (inst::ui::mainApp->CreateShowDialog("sig.uninstall_complete"_lang, "sig.complete_desc"_lang, {"sig.restart"_lang, "sig.later"_lang}, false) == 0) bpcRebootSystem();
                }
                else inst::ui::mainApp->CreateShowDialog("sig.remove_failed"_lang, "sig.remove_failed_desc"_lang, {"common.ok"_lang}, true);
            } else return;
        }
        catch (std::exception& e)
        {
            LOG_DEBUG("Failed to install Signature Patches");
            LOG_DEBUG("%s", e.what());
            fprintf(stdout, "%s", e.what());
            inst::ui::mainApp->CreateShowDialog("sig.generic_error"_lang, (std::string)e.what(), {"common.ok"_lang}, true);
        }
        bpcExit();
    }
}