#include "ui/MainApplication.hpp"
#include "util/curl.hpp"
#include "util/util.hpp"
#include "util/unzip.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;
}

namespace sig {
    void installSigPatches () {
        int ourResult = inst::ui::mainApp->CreateShowDialog("Install signature patches?", "Signature patches are required for installing and playing NSP contents!", {"Install", "Uninstall", "Cancel"}, true);
        if (ourResult == 0) {
            if (!util::copyFile("sdmc:/bootloader/patches.ini", "sdmc:/bootloader/patches.ini.old")) {
                if (inst::ui::mainApp->CreateShowDialog("Could not back up old Hekate patches.ini! Install anyway?", "", {"Yes", "No"}, false)) return;
            }
            std::string ourPath = appVariables::appDir + "patches.zip";
            bool didDownload = curlStuff::downloadFile("http://github.com/Joonie86/hekate/releases/download/5.0.0J/Kosmos_patches_10_09_2019.zip", ourPath.c_str());
            bool didExtract = false;
            if (didDownload) didExtract = zipStuff::extractFile(ourPath, "sdmc:/");
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
            if (!util::copyFile( "sdmc:/bootloader/patches.ini.old", "sdmc:/bootloader/patches.ini")) {
                if (inst::ui::mainApp->CreateShowDialog("Unable to restore original Hekate patches.ini! Continue uninstalling?", "", {"Yes", "No"}, false)) return;
            } else std::filesystem::remove("sdmc:/bootloader/patches.ini.old");
            if (util::removeDirectory("sdmc:/atmosphere/exefs_patches/es_patches")) inst::ui::mainApp->CreateShowDialog("Uninstall complete", "Restart your console to apply", {"OK"}, true);
            else inst::ui::mainApp->CreateShowDialog("Unable to remove signature patches", "Files may have been renamed or deleted", {"OK"}, true);
        } else return;
    }
}