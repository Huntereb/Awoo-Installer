#include "ui/MainApplication.hpp"
#include "util/curl.hpp"
#include "util/util.hpp"
#include "util/unzip.hpp"
#include "util/config.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;
}

namespace sig {
    void installSigPatches () {
        try {
            std::string patchesVersion = inst::util::readTextFromFile("sdmc:/atmosphere/exefs_patches/es_patches/patches.txt");
            std::string versionText = "";
            std::string installButtonText = "Install";
            if (patchesVersion != "") {
                versionText = "\n\nYou currently have signature patches installed for HOS version " + patchesVersion + ".";
                installButtonText = "Update";
            }
            int ourResult = inst::ui::mainApp->CreateShowDialog("Install signature patches?", "Signature patches are required for installing and playing NSP contents." + versionText, {installButtonText, "Uninstall", "Cancel"}, true);
            if (ourResult == 0) {
                if (!inst::util::copyFile("sdmc:/bootloader/patches.ini", inst::config::appDir + "/patches.ini.old")) {
                    if (inst::ui::mainApp->CreateShowDialog("Could not back up old Hekate patches.ini! Install anyway?", "Installing patches requires the use of the Hekate bootloader.", {"Yes", "No"}, false)) return;
                }
                std::string ourPath = inst::config::appDir + "/patches.zip";
                bool didDownload = inst::curl::downloadFile(inst::config::sigPatchesUrl, ourPath.c_str());
                bool didExtract = false;
                if (didDownload) didExtract = inst::zip::extractFile(ourPath, "sdmc:/");
                else {
                    inst::ui::mainApp->CreateShowDialog("Network connection not available", "Check that airplane mode is disabled and you're connected to a local network.", {"OK"}, true);
                    return;
                }
                std::filesystem::remove(ourPath);
                if (didExtract) {
                    patchesVersion = inst::util::readTextFromFile("sdmc:/atmosphere/exefs_patches/es_patches/patches.txt");
                    versionText = "";
                    if (patchesVersion != "") versionText = "Your signature patches have been updated for HOS version " + patchesVersion + "! ";
                    inst::ui::mainApp->CreateShowDialog("Install complete!", versionText + "\n\nRestart your console to apply!", {"OK"}, true);
                }
                else {
                    inst::ui::mainApp->CreateShowDialog("Could not extract files!", "", {"OK"}, true);
                    return;
                }
                return;
            } else if (ourResult == 1) {
                if (!inst::util::copyFile( inst::config::appDir + "/patches.ini.old", "sdmc:/bootloader/patches.ini")) {
                    if (inst::ui::mainApp->CreateShowDialog("Unable to restore original Hekate patches.ini! Continue uninstalling?", "", {"Yes", "No"}, false)) return;
                } else std::filesystem::remove(inst::config::appDir + "/patches.ini.old");
                if (inst::util::removeDirectory("sdmc:/atmosphere/exefs_patches/es_patches")) inst::ui::mainApp->CreateShowDialog("Uninstall complete", "Restart your console to apply", {"OK"}, true);
                else inst::ui::mainApp->CreateShowDialog("Unable to remove signature patches", "Files may have been renamed or deleted", {"OK"}, true);
            } else return;
        }
        catch (std::exception& e)
        {
            printf("Failed to install Signature Patches");
            printf("%s", e.what());
            fprintf(stdout, "%s", e.what());
            inst::ui::mainApp->CreateShowDialog("Failed to install Signature Patches!", (std::string)e.what(), {"OK"}, true);
        }
    }
}