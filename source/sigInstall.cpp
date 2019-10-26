#include "ui/MainApplication.hpp"
#include "util/curl.hpp"
#include "util/util.hpp"
#include "util/unzip.hpp"
#include "config.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;
}

namespace sig {
    void installSigPatches () {
        int ourResult = inst::ui::mainApp->CreateShowDialog("Install signature patches?", "Signature patches are required for installing and playing NSP contents!", {"Install", "Uninstall", "Cancel"}, true);
        if (ourResult == 0) {
            if (!inst::util::copyFile("sdmc:/bootloader/patches.ini", "sdmc:/bootloader/patches.ini.old")) {
                if (inst::ui::mainApp->CreateShowDialog("Could not back up old Hekate patches.ini! Install anyway?", "", {"Yes", "No"}, false)) return;
            }
            std::string ourPath = inst::config::appDir + "patches.zip";
            bool didDownload = inst::curl::downloadFile("http://github.com/Joonie86/hekate/releases/download/5.0.0J/Kosmos_patches_10_09_2019.zip", ourPath.c_str());
            bool didExtract = false;
            if (didDownload) didExtract = inst::zip::extractFile(ourPath, "sdmc:/");
            else {
                inst::ui::mainApp->CreateShowDialog("Could not download signature patches!", "Check your internet connection and try again", {"OK"}, true);
                return;
            }
            std::filesystem::remove(ourPath);
            if (didExtract) inst::ui::mainApp->CreateShowDialog("Install complete!", "Restart your console to apply!", {"OK"}, true);
            else {
                inst::ui::mainApp->CreateShowDialog("Could not extract files!", "", {"OK"}, true);
                return;
            }
            return;
        } else if (ourResult == 1) {
            if (!inst::util::copyFile( "sdmc:/bootloader/patches.ini.old", "sdmc:/bootloader/patches.ini")) {
                if (inst::ui::mainApp->CreateShowDialog("Unable to restore original Hekate patches.ini! Continue uninstalling?", "", {"Yes", "No"}, false)) return;
            } else std::filesystem::remove("sdmc:/bootloader/patches.ini.old");
            if (inst::util::removeDirectory("sdmc:/atmosphere/exefs_patches/es_patches")) inst::ui::mainApp->CreateShowDialog("Uninstall complete", "Restart your console to apply", {"OK"}, true);
            else inst::ui::mainApp->CreateShowDialog("Unable to remove signature patches", "Files may have been renamed or deleted", {"OK"}, true);
        } else return;
    }
}