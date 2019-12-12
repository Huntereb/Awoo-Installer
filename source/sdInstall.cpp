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
#include <thread>

#include "install/install_nsp.hpp"
#include "install/install_xci.hpp"
#include "install/local_xci.hpp"
#include "nx/fs.hpp"
#include "util/file_util.hpp"
#include "util/title_util.hpp"
#include "util/error.hpp"

#include "ui/MainApplication.hpp"
#include "sdInstall.hpp"
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
        mainApp->instpage->awooImage->SetVisible(!inst::config::gayMode);
        mainApp->LoadLayout(mainApp->instpage);
        mainApp->CallForRender();
    }
}

namespace nspInstStuff {

    std::string finishedMessage() {
        std::vector<std::string> finishMessages = {"Enjoy your \"legal backups\"!", "I'm sure after you give the game a try you'll have tons of fun actually buying it!", "You buy gamu right? Nintendo-san thanka-you for your purchase!", "Bypassing DRM is great, isn't it?", "You probably saved like six trees by not buying the game! All that plastic goes somewhere!", "Nintendo ninjas have been dispatched to your current location.", "And we didn't even have to shove a political ideology down your throat to get here!"};
        srand(time(NULL));
        return(finishMessages[rand() % finishMessages.size()]);
    }

    void installNspFromFile(std::vector<std::filesystem::path> ourTitleList, int whereToInstall)
    {
        inst::util::initInstallServices();
        inst::ui::loadInstallScreen();
        bool nspInstalled = true;
        NcmStorageId m_destStorageId = NcmStorageId_SdCard;
        std::vector<std::string> filesToBeRenamed = {};
        std::vector<std::string> oldNamesOfFiles = {};

        if (whereToInstall) m_destStorageId = NcmStorageId_BuiltInUser;
        unsigned int titleItr;

        std::vector<int> previousClockValues;
        if (inst::config::overClock) {
            previousClockValues.push_back(inst::util::setClockSpeed(0, 1785000000)[0]);
            previousClockValues.push_back(inst::util::setClockSpeed(1, 76800000)[0]);
            previousClockValues.push_back(inst::util::setClockSpeed(2, 1600000000)[0]);
        }

        try
        {
            for (titleItr = 0; titleItr < ourTitleList.size(); titleItr++) {
                inst::ui::setTopInstInfoText("Installing " + inst::util::shortenString(ourTitleList[titleItr].filename().string(), 40, true) + " from SD card");
                tin::install::Install* installTask;

                if (ourTitleList[titleItr].extension() == ".xci" || ourTitleList[titleItr].extension() == ".xcz") {
                    auto localXCI = new tin::install::xci::LocalXCI(ourTitleList[titleItr]);
                    installTask = new tin::install::xci::XCIInstallTask(m_destStorageId, inst::config::ignoreReqVers, localXCI);

                    inst::ui::setInstInfoText("Preparing installation...");
                    inst::ui::setInstBarPerc(0);
                    installTask->Prepare();
                    installTask->Begin();
                } else {
                    if (ourTitleList[titleItr].extension() == ".nsz") {
                        oldNamesOfFiles.push_back(ourTitleList[titleItr]);
                        std::string newfilename = ourTitleList[titleItr].string().substr(0, ourTitleList[titleItr].string().find_last_of('.'))+"_temp.nsp";
                        rename(ourTitleList[titleItr], newfilename);
                        filesToBeRenamed.push_back(newfilename);
                        ourTitleList[titleItr] = newfilename;
                    }
                    
                    std::string path = "@Sdcard://" + ourTitleList[titleItr].string().erase(0, 6);

                    nx::fs::IFileSystem fileSystem;
                    fileSystem.OpenFileSystemWithId(path, FsFileSystemType_ApplicationPackage, 0);
                    tin::install::nsp::SimpleFileSystem simpleFS(fileSystem, "/", path + "/");
                    installTask = new tin::install::nsp::NSPInstallTask(simpleFS, m_destStorageId, inst::config::ignoreReqVers);

                    inst::ui::setInstInfoText("Preparing installation...");
                    inst::ui::setInstBarPerc(0);
                    installTask->Prepare();
                    installTask->Begin();
                }
            }
        }
        catch (std::exception& e)
        {
            LOG_DEBUG("Failed to install");
            LOG_DEBUG("%s", e.what());
            fprintf(stdout, "%s", e.what());
            inst::ui::setInstInfoText("Failed to install " + inst::util::shortenString(ourTitleList[titleItr].filename().string(), 42, true));
            inst::ui::setInstBarPerc(0);
            std::thread audioThread(inst::util::playAudio,"romfs:/audio/bark.wav");
            inst::ui::mainApp->CreateShowDialog("Failed to install " + inst::util::shortenString(ourTitleList[titleItr].filename().string(), 42, true) + "!", "Partially installed contents can be removed from the System Settings applet.\n\n" + (std::string)e.what(), {"OK"}, true);
            audioThread.join();
            nspInstalled = false;
        }

        if (previousClockValues.size() > 0) {
            inst::util::setClockSpeed(0, previousClockValues[0]);
            inst::util::setClockSpeed(1, previousClockValues[1]);
            inst::util::setClockSpeed(2, previousClockValues[2]);
        }

        for (unsigned int i = 0; i < filesToBeRenamed.size(); i++) {
            if (ourTitleList.size() == 1) ourTitleList[0] = oldNamesOfFiles[i];
            if (std::filesystem::exists(filesToBeRenamed[i])) {
                rename(filesToBeRenamed[i].c_str(), oldNamesOfFiles[i].c_str());
            }
        }

        if(nspInstalled) {
            inst::ui::setInstInfoText("Install complete");
            inst::ui::setInstBarPerc(100);
            std::thread audioThread(inst::util::playAudio,"romfs:/audio/awoo.wav");
            if (ourTitleList.size() > 1) {
                if (inst::config::deletePrompt) {
                    if(inst::ui::mainApp->CreateShowDialog(std::to_string(ourTitleList.size()) + " files installed successfully! Delete them from the SD card?", "The original files aren't needed anymore after they've been installed", {"No","Yes"}, false) == 1) {
                        for (long unsigned int i = 0; i < ourTitleList.size(); i++) {
                            if (std::filesystem::exists(ourTitleList[i])) std::filesystem::remove(ourTitleList[i]);
                        }
                    }
                } else inst::ui::mainApp->CreateShowDialog(std::to_string(ourTitleList.size()) + " files installed successfully!", nspInstStuff::finishedMessage(), {"OK"}, true);
            } else {
                if (inst::config::deletePrompt) {
                    if(inst::ui::mainApp->CreateShowDialog(inst::util::shortenString(ourTitleList[0].filename().string(), 32, true) + " installed! Delete it from the SD card?", "The original file isn't needed anymore after it's been installed", {"No","Yes"}, false) == 1) if (std::filesystem::exists(ourTitleList[0])) std::filesystem::remove(ourTitleList[0]);
                } else inst::ui::mainApp->CreateShowDialog(inst::util::shortenString(ourTitleList[0].filename().string(), 42, true) + " installed!", nspInstStuff::finishedMessage(), {"OK"}, true);
            }
            audioThread.join();
        }

        LOG_DEBUG("Done");
        inst::ui::loadMainMenu();
        inst::util::deinitInstallServices();
        return;
    }
}
