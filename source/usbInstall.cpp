#include <string>
#include "util/error.hpp"
#include "usbInstall.hpp"
#include "install/usb_nsp.hpp"
#include "install/install_nsp_remote.hpp"
#include "util/usb_util.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "ui/usbInstPage.hpp"

#include "install/usb_xci.hpp"
#include "install/install_xci.hpp"
#include "sdInstall.hpp"
#include "ui/MainApplication.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;

    void setUsbInfoText(std::string ourText){
        mainApp->usbinstPage->pageInfoText->SetText(ourText);
        mainApp->CallForRender();
    }
}

namespace usbInstStuff {
    struct TUSHeader
    {
        u32 magic; // TUL0 (Tinfoil Usb List 0)
        u32 titleListSize;
        u64 padding;
    } PACKED;

    std::vector<std::string> OnSelected() {
        Result rc = 0;

        while(true) {
            rc = usbDsWaitReady(1000000);
            if (R_SUCCEEDED(rc)) break;
            else if ((rc & 0x3FFFFF) != 0xEA01)
                return {};
        }

        TUSHeader header;
        tin::util::USBRead(&header, sizeof(TUSHeader));

        if (header.magic != 0x304C5554)
            return {};

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
        inst::ui::loadInstallScreen();
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
                inst::ui::setTopInstInfoText("Installing " + fileNames[fileItr] + " over USB");

                tin::install::Install* installTask;

                if (ourTitleList[fileItr].compare(ourTitleList[fileItr].size() - 3, 2, "xc") == 0) {
                    auto usbXCI = new tin::install::xci::USBXCI(ourTitleList[fileItr]);
                    installTask = new tin::install::xci::XCIInstallTask(m_destStorageId, inst::config::ignoreReqVers, usbXCI);
                } else {
                    auto usbNSP = new tin::install::nsp::USBNSP(ourTitleList[fileItr]);
                    installTask = new tin::install::nsp::RemoteNSPInstall(m_destStorageId, inst::config::ignoreReqVers, usbNSP);
                }

                LOG_DEBUG("%s\n", "Preparing installation");
                inst::ui::setInstInfoText("Preparing installation...");
                inst::ui::setInstBarPerc(0);
                installTask->Prepare();

                installTask->Begin();
            }
        }
        catch (std::exception& e) {
            LOG_DEBUG("Failed to install");
            LOG_DEBUG("%s", e.what());
            fprintf(stdout, "%s", e.what());
            inst::ui::setInstInfoText("Failed to install " + fileNames[fileItr]);
            inst::ui::setInstBarPerc(0);
            inst::ui::mainApp->CreateShowDialog("Failed to install " + fileNames[fileItr] + "!", "Partially installed contents can be removed from the System Settings applet.\n\n" + (std::string)e.what(), {"OK"}, true);
            nspInstalled = false;
        }

        if (previousClockValues.size() > 0) {
            inst::util::setClockSpeed(0, previousClockValues[0]);
            inst::util::setClockSpeed(1, previousClockValues[1]);
            inst::util::setClockSpeed(2, previousClockValues[2]);
        }


        if(nspInstalled) {
            tin::util::USBCmdManager::SendExitCmd();
            inst::ui::setInstInfoText("Install complete");
            inst::ui::setInstBarPerc(100);
            if (ourTitleList.size() > 1) inst::ui::mainApp->CreateShowDialog(std::to_string(ourTitleList.size()) + " files installed successfully!", nspInstStuff::finishedMessage(), {"OK"}, true);
            else inst::ui::mainApp->CreateShowDialog(fileNames[0] + " installed!", nspInstStuff::finishedMessage(), {"OK"}, true);
        }
        
        LOG_DEBUG("Done");
        inst::ui::loadMainMenu();
        inst::util::deinitInstallServices();
        return;
    }
}