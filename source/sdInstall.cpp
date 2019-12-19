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
#include <memory>
#include "sdInstall.hpp"
#include "install/install_nsp.hpp"
#include "install/install_xci.hpp"
#include "install/sdmc_xci.hpp"
#include "install/sdmc_nsp.hpp"
#include "nx/fs.hpp"
#include "util/file_util.hpp"
#include "util/title_util.hpp"
#include "util/error.hpp"
#include "util/config.hpp"
#include "util/util.hpp"
#include "ui/MainApplication.hpp"
#include "ui/instPage.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;
}

namespace nspInstStuff {

    void installNspFromFile(std::vector<std::filesystem::path> ourTitleList, int whereToInstall)
    {
        inst::util::initInstallServices();
        inst::ui::instPage::loadInstallScreen();
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
                inst::ui::instPage::setTopInstInfoText("Installing " + inst::util::shortenString(ourTitleList[titleItr].filename().string(), 40, true) + " from SD card");
                std::unique_ptr<tin::install::Install> installTask;

                if (ourTitleList[titleItr].extension() == ".xci" || ourTitleList[titleItr].extension() == ".xcz") {
                    auto sdmcXCI = std::make_shared<tin::install::xci::SDMCXCI>(ourTitleList[titleItr]);
                    installTask = std::make_unique<tin::install::xci::XCIInstallTask>(m_destStorageId, inst::config::ignoreReqVers, sdmcXCI);
                } else {
                    auto sdmcNSP = std::make_shared<tin::install::nsp::SDMCNSP>(ourTitleList[titleItr]);
                    installTask = std::make_unique<tin::install::nsp::NSPInstall>(m_destStorageId, inst::config::ignoreReqVers, sdmcNSP);
                }

                LOG_DEBUG("%s\n", "Preparing installation");
                inst::ui::instPage::setInstInfoText("Preparing installation...");
                inst::ui::instPage::setInstBarPerc(0);
                installTask->Prepare();
                installTask->Begin();
            }
        }
        catch (std::exception& e)
        {
            LOG_DEBUG("Failed to install");
            LOG_DEBUG("%s", e.what());
            fprintf(stdout, "%s", e.what());
            inst::ui::instPage::setInstInfoText("Failed to install " + inst::util::shortenString(ourTitleList[titleItr].filename().string(), 42, true));
            inst::ui::instPage::setInstBarPerc(0);
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
            inst::ui::instPage::setInstInfoText("Install complete");
            inst::ui::instPage::setInstBarPerc(100);
            std::thread audioThread(inst::util::playAudio,"romfs:/audio/awoo.wav");
            if (ourTitleList.size() > 1) {
                if (inst::config::deletePrompt) {
                    if(inst::ui::mainApp->CreateShowDialog(std::to_string(ourTitleList.size()) + " files installed successfully! Delete them from the SD card?", "The original files aren't needed anymore after they've been installed", {"No","Yes"}, false) == 1) {
                        for (long unsigned int i = 0; i < ourTitleList.size(); i++) {
                            if (std::filesystem::exists(ourTitleList[i])) std::filesystem::remove(ourTitleList[i]);
                        }
                    }
                } else inst::ui::mainApp->CreateShowDialog(std::to_string(ourTitleList.size()) + " files installed successfully!", inst::ui::instPage::finishedMessage(), {"OK"}, true);
            } else {
                if (inst::config::deletePrompt) {
                    if(inst::ui::mainApp->CreateShowDialog(inst::util::shortenString(ourTitleList[0].filename().string(), 32, true) + " installed! Delete it from the SD card?", "The original file isn't needed anymore after it's been installed", {"No","Yes"}, false) == 1) if (std::filesystem::exists(ourTitleList[0])) std::filesystem::remove(ourTitleList[0]);
                } else inst::ui::mainApp->CreateShowDialog(inst::util::shortenString(ourTitleList[0].filename().string(), 42, true) + " installed!", inst::ui::instPage::finishedMessage(), {"OK"}, true);
            }
            audioThread.join();
        }

        LOG_DEBUG("Done");
        inst::ui::instPage::loadMainMenu();
        inst::util::deinitInstallServices();
        return;
    }
}
