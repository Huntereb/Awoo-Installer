/*
Copyright (c) 2017-2018 Adubbz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <cstring>
#include <sstream>
#include <filesystem>
#include <ctime>

#include "install/install_nsp.hpp"
#include "nx/fs.hpp"
#include "util/file_util.hpp"
#include "util/title_util.hpp"
#include "util/error.hpp"

#include "ui/MainApplication.hpp"
#include "nspInstall.hpp"
#include "util/config.hpp"
#include "util/util.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;

    void setTopInstInfoText(std::string ourText){
        mainApp->instpage->pageInfoText->SetText(ourText);
        mainApp->CallForRender();
    }

    void setInstInfoText(std::string ourText){
        mainApp->instpage->installInfoText->SetText(ourText);
        mainApp->CallForRender();
    }

    void setInstBarPerc(double ourPercent){
        mainApp->instpage->installBar->SetVisible(true);
        mainApp->instpage->installBar->SetProgress(ourPercent);
        mainApp->CallForRender();
    }

    void loadMainMenu(){
        mainApp->LoadLayout(mainApp->mainPage);
    }

    void loadInstallScreen(){
        mainApp->instpage->pageInfoText->SetText("");
        mainApp->instpage->installInfoText->SetText("");
        mainApp->instpage->installBar->SetProgress(0);
        mainApp->instpage->installBar->SetVisible(false);
        if (inst::config::gayMode) mainApp->instpage->awooImage->SetVisible(false);
        else mainApp->instpage->awooImage->SetVisible(true);
        mainApp->LoadLayout(mainApp->instpage);
        mainApp->CallForRender();
    }
}

namespace nspInstStuff {

    std::string finishedMessage() {
        std::vector<std::string> finishMessages = {"Enjoy your \"legal backups\"!", "I'm sure after you give the game a try you'll have tons of fun actually buying it!", "You buy gamu right? Nintendo-san thanka-you for your purchase!", "Bypassing DRM is great, isn't it?", "You probably saved like six trees by not buying the game! All that plastic goes somewhere!"};
        srand(time(NULL));
        return(finishMessages[rand() % finishMessages.size()]);
    }

    void installNspFromFile(std::vector<std::filesystem::path> ourNspList, int whereToInstall)
    {
        inst::util::initInstallServices();
        if (appletGetAppletType() == AppletType_Application || appletGetAppletType() == AppletType_SystemApplication) appletBeginBlockingHomeButton(0);
        inst::ui::loadInstallScreen();
        bool nspInstalled = true;
        NcmStorageId m_destStorageId = NcmStorageId_SdCard;
        std::vector<std::string> filesToBeRenamed = {};
        std::vector<std::string> oldNamesOfFiles = {};

        if (whereToInstall) m_destStorageId = NcmStorageId_BuiltInUser;
        unsigned int nspItr;

        std::vector<int> previousClockValues;
        if (inst::config::overClock) {
            previousClockValues.push_back(inst::util::setClockSpeed(0, 1785000000)[0]);
            previousClockValues.push_back(inst::util::setClockSpeed(1, 76800000)[0]);
            previousClockValues.push_back(inst::util::setClockSpeed(2, 1600000000)[0]);
        }

        try
        {
            for (nspItr = 0; nspItr < ourNspList.size(); nspItr++) {
                inst::ui::setTopInstInfoText("Installing " + inst::util::shortenString(ourNspList[nspItr].string().erase(0, 6), 42, true));
                
                if (ourNspList[nspItr].extension() == ".nsz") {
                    oldNamesOfFiles.push_back(ourNspList[nspItr]);
                    std::string newfilename = ourNspList[nspItr].string().substr(0, ourNspList[nspItr].string().find_last_of('.'))+"_temp.nsp";
                    rename(ourNspList[nspItr], newfilename);
                    filesToBeRenamed.push_back(newfilename);
                    ourNspList[nspItr] = newfilename;
                }

                std::string path = "@Sdcard://" + ourNspList[nspItr].string().erase(0, 6);

                nx::fs::IFileSystem fileSystem;
                fileSystem.OpenFileSystemWithId(path, FsFileSystemType_ApplicationPackage, 0);
                tin::install::nsp::SimpleFileSystem simpleFS(fileSystem, "/", path + "/");
                tin::install::nsp::NSPInstallTask task(simpleFS, m_destStorageId, inst::config::ignoreReqVers);

                printf("Preparing installation\n");
                inst::ui::setInstInfoText("Preparing installation...");
                inst::ui::setInstBarPerc(0);
                task.Prepare();

                task.Begin();
            }
        }
        catch (std::exception& e)
        {
            printf("Failed to install");
            printf("%s", e.what());
            fprintf(stdout, "%s", e.what());
            inst::ui::setInstInfoText("Failed to install " + inst::util::shortenString(ourNspList[nspItr].string().erase(0, 6), 42, true));
            inst::ui::setInstBarPerc(0);
            inst::ui::mainApp->CreateShowDialog("Failed to install " + inst::util::shortenString(ourNspList[nspItr].string().erase(0, 6), 42, true) + "!", "Partially installed contents can be removed from the System Settings applet.\n\n" + (std::string)e.what(), {"OK"}, true);
            nspInstalled = false;
        }

        if (previousClockValues.size() > 0) {
            inst::util::setClockSpeed(0, previousClockValues[0]);
            inst::util::setClockSpeed(1, previousClockValues[1]);
            inst::util::setClockSpeed(2, previousClockValues[2]);
        }

        for (unsigned int i = 0; i < filesToBeRenamed.size(); i++) {
            if (ourNspList.size() == 1) ourNspList[0] = oldNamesOfFiles[i];
            if (std::filesystem::exists(filesToBeRenamed[i])) {
                rename(filesToBeRenamed[i].c_str(), oldNamesOfFiles[i].c_str());
            }
        }

        if(nspInstalled) {
            inst::ui::setInstInfoText("Install complete");
            inst::ui::setInstBarPerc(100);
            if (ourNspList.size() > 1) {
                if (inst::config::deletePrompt) {
                    if(inst::ui::mainApp->CreateShowDialog(std::to_string(ourNspList.size()) + " files installed successfully! Delete them from the SD card?", "The original files aren't needed anymore after they've been installed", {"No","Yes"}, false) == 1) {
                        for (long unsigned int i = 0; i < ourNspList.size(); i++) {
                            if (std::filesystem::exists(ourNspList[i])) std::filesystem::remove(ourNspList[i]);
                        }
                    }
                } else inst::ui::mainApp->CreateShowDialog(std::to_string(ourNspList.size()) + " files installed successfully!", nspInstStuff::finishedMessage(), {"OK"}, true);
            } else {
                if (inst::config::deletePrompt) {
                    if(inst::ui::mainApp->CreateShowDialog(inst::util::shortenString(ourNspList[0].string().erase(0, 6), 32, true) + " installed! Delete it from the SD card?", "The original file isn't needed anymore after it's been installed", {"No","Yes"}, false) == 1) if (std::filesystem::exists(ourNspList[0])) std::filesystem::remove(ourNspList[0]);
                } else inst::ui::mainApp->CreateShowDialog(inst::util::shortenString(ourNspList[0].string().erase(0, 6), 42, true) + " installed!", nspInstStuff::finishedMessage(), {"OK"}, true);
            }
        }

        printf("Done");
        if (appletGetAppletType() == AppletType_Application || appletGetAppletType() == AppletType_SystemApplication) appletEndBlockingHomeButton();
        inst::ui::loadMainMenu();
        inst::util::deinitInstallServices();
        return;
    }
}
