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

#include <string>
#include <thread>
#include "usbInstall.hpp"
#include "install/usb_nsp.hpp"
#include "install/install_nsp.hpp"
#include "install/usb_xci.hpp"
#include "install/install_xci.hpp"
#include "util/error.hpp"
#include "util/usb_util.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/lang.hpp"
#include "ui/MainApplication.hpp"
#include "ui/usbInstPage.hpp"
#include "ui/instPage.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;
}

namespace usbInstStuff {
    struct TUSHeader
    {
        u32 magic; // TUL0 (Tinfoil Usb List 0)
        u32 titleListSize;
        u64 padding;
    } PACKED;

    std::vector<std::string> OnSelected() {
        TUSHeader header;
        while(true) {
            if (tin::util::USBRead(&header, sizeof(TUSHeader), 500000000) != 0) break;
            hidScanInput();
            u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
            if (kDown & KEY_B) return {};
            if (kDown & KEY_X) inst::ui::mainApp->CreateShowDialog("inst.usb.help.title"_lang, "inst.usb.help.desc"_lang, {"common.ok"_lang}, true);
            if (inst::util::getUsbState() != 5) return {};
        }

        if (header.magic != 0x304C5554) return {};

        auto titleListBuf = std::make_unique<char[]>(header.titleListSize+1);
        std::vector<std::string> titleNames;
        memset(titleListBuf.get(), 0, header.titleListSize+1);

        tin::util::USBRead(titleListBuf.get(), header.titleListSize);

        // Split the string up into individual title names
        std::stringstream titleNamesStream(titleListBuf.get());
        std::string segment;
        while (std::getline(titleNamesStream, segment, '\n')) {
            titleNames.push_back(segment);
        }

        return titleNames;
    }

    void installTitleUsb(std::vector<std::string> ourTitleList, int ourStorage)
    {
        inst::util::initInstallServices();
        inst::ui::instPage::loadInstallScreen();
        bool nspInstalled = true;
        NcmStorageId m_destStorageId = NcmStorageId_SdCard;

        if (ourStorage) m_destStorageId = NcmStorageId_BuiltInUser;
        unsigned int fileItr;

        std::vector<std::string> fileNames;
        for (long unsigned int i = 0; i < ourTitleList.size(); i++) {
            fileNames.push_back(inst::util::shortenString(inst::util::formatUrlString(ourTitleList[i]), 40, true));
        }

        std::vector<int> previousClockValues;
        if (inst::config::overClock) {
            previousClockValues.push_back(inst::util::setClockSpeed(0, 1785000000)[0]);
            previousClockValues.push_back(inst::util::setClockSpeed(1, 76800000)[0]);
            previousClockValues.push_back(inst::util::setClockSpeed(2, 1600000000)[0]);
        }

        try {
            for (fileItr = 0; fileItr < ourTitleList.size(); fileItr++) {
                inst::ui::instPage::setTopInstInfoText("inst.info_page.top_info0"_lang + fileNames[fileItr] + "inst.usb.source_string"_lang);
                std::unique_ptr<tin::install::Install> installTask;

                if (ourTitleList[fileItr].compare(ourTitleList[fileItr].size() - 3, 2, "xc") == 0) {
                    auto usbXCI = std::make_shared<tin::install::xci::USBXCI>(ourTitleList[fileItr]);
                    installTask = std::make_unique<tin::install::xci::XCIInstallTask>(m_destStorageId, inst::config::ignoreReqVers, usbXCI);
                } else {
                    auto usbNSP = std::make_shared<tin::install::nsp::USBNSP>(ourTitleList[fileItr]);
                    installTask = std::make_unique<tin::install::nsp::NSPInstall>(m_destStorageId, inst::config::ignoreReqVers, usbNSP);
                }

                LOG_DEBUG("%s\n", "Preparing installation");
                inst::ui::instPage::setInstInfoText("inst.info_page.preparing"_lang);
                inst::ui::instPage::setInstBarPerc(0);
                installTask->Prepare();

                installTask->Begin();
            }
        }
        catch (std::exception& e) {
            LOG_DEBUG("Failed to install");
            LOG_DEBUG("%s", e.what());
            fprintf(stdout, "%s", e.what());
            inst::ui::instPage::setInstInfoText("inst.info_page.failed"_lang + fileNames[fileItr]);
            inst::ui::instPage::setInstBarPerc(0);
            std::thread audioThread(inst::util::playAudio,"romfs:/audio/bark.wav");
            inst::ui::mainApp->CreateShowDialog("inst.info_page.failed"_lang + fileNames[fileItr] + "!", "inst.info_page.failed_desc"_lang + "\n\n" + (std::string)e.what(), {"common.ok"_lang}, true);
            audioThread.join();
            nspInstalled = false;
        }

        if (previousClockValues.size() > 0) {
            inst::util::setClockSpeed(0, previousClockValues[0]);
            inst::util::setClockSpeed(1, previousClockValues[1]);
            inst::util::setClockSpeed(2, previousClockValues[2]);
        }

        if(nspInstalled) {
            tin::util::USBCmdManager::SendExitCmd();
            inst::ui::instPage::setInstInfoText("inst.info_page.complete"_lang);
            inst::ui::instPage::setInstBarPerc(100);
            std::thread audioThread(inst::util::playAudio,"romfs:/audio/awoo.wav");
            if (ourTitleList.size() > 1) inst::ui::mainApp->CreateShowDialog(std::to_string(ourTitleList.size()) + "inst.info_page.desc0"_lang, Language::GetRandomMsg(), {"common.ok"_lang}, true);
            else inst::ui::mainApp->CreateShowDialog(fileNames[0] + "inst.info_page.desc1"_lang, Language::GetRandomMsg(), {"common.ok"_lang}, true);
            audioThread.join();
        }
        
        LOG_DEBUG("Done");
        inst::ui::instPage::loadMainMenu();
        inst::util::deinitInstallServices();
        return;
    }
}