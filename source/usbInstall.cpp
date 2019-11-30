#include <string>
#include "usbInstall.hpp"
#include "install/usb_nsp.hpp"
#include "install/install_nsp_remote.hpp"
#include "util/usb_util.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "ui/usbInstPage.hpp"

#include "nspInstall.hpp"
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
        u32 nspListSize;
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

        auto nspListBuf = std::make_unique<char[]>(header.nspListSize+1);
        std::vector<std::string> nspNames;
        memset(nspListBuf.get(), 0, header.nspListSize+1);

        tin::util::USBRead(nspListBuf.get(), header.nspListSize);

        // Split the string up into individual nsp names
        std::stringstream nspNameStream(nspListBuf.get());
        std::string segment;
        while (std::getline(nspNameStream, segment, '\n')) {
            nspNames.push_back(segment);
        }

        return nspNames;
    }

    void installNspUsb(std::vector<std::string> ourNspList, int ourStorage)
    {
        inst::util::initInstallServices();
        inst::ui::loadInstallScreen();
        bool nspInstalled = true;
        NcmStorageId m_destStorageId = NcmStorageId_SdCard;

        if (ourStorage) m_destStorageId = NcmStorageId_BuiltInUser;
        unsigned int fileItr;

        std::vector<std::string> fileNames;
        for (long unsigned int i = 0; i < ourNspList.size(); i++) {
            fileNames.push_back(inst::util::shortenString(ourNspList[i], 42, true));
        }
/*
        std::vector<int> previousClockValues;
        if (inst::config::overClock) {
            previousClockValues.push_back(inst::util::setClockSpeed(0, 1785000000)[0]);
            previousClockValues.push_back(inst::util::setClockSpeed(1, 76800000)[0]);
            previousClockValues.push_back(inst::util::setClockSpeed(2, 1600000000)[0]);
        }
*/
        try {
            for (fileItr = 0; fileItr < ourNspList.size(); fileItr++) {
                inst::ui::setTopInstInfoText("Installing " + fileNames[fileItr]);

                tin::install::nsp::USBNSP usbNSP(ourNspList[fileItr]);
                tin::install::nsp::RemoteNSPInstall install(m_destStorageId, inst::config::ignoreReqVers, &usbNSP);

                printf("%s\n", "Preparing installation");
                inst::ui::setInstInfoText("Preparing installation...");
                inst::ui::setInstBarPerc(0);
                install.Prepare();

                install.Begin();
            }
        }
        catch (std::exception& e) {
            printf("Failed to install");
            printf("%s", e.what());
            fprintf(stdout, "%s", e.what());
            inst::ui::setInstInfoText("Failed to install " + fileNames[fileItr]);
            inst::ui::setInstBarPerc(0);
            inst::ui::mainApp->CreateShowDialog("Failed to install " + fileNames[fileItr] + "!", "Partially installed contents can be removed from the System Settings applet.\n\n" + (std::string)e.what(), {"OK"}, true);
            nspInstalled = false;
        }

        tin::util::USBCmdManager::SendExitCmd();
/*
        if (previousClockValues.size() > 0) {
            inst::util::setClockSpeed(0, previousClockValues[0]);
            inst::util::setClockSpeed(1, previousClockValues[1]);
            inst::util::setClockSpeed(2, previousClockValues[2]);
        }
*/

        if(nspInstalled) {
            inst::ui::setInstInfoText("Install complete");
            inst::ui::setInstBarPerc(100);
            if (ourNspList.size() > 1) inst::ui::mainApp->CreateShowDialog(std::to_string(ourNspList.size()) + " files installed successfully!", nspInstStuff::finishedMessage(), {"OK"}, true);
            else inst::ui::mainApp->CreateShowDialog(fileNames[0] + " installed!", nspInstStuff::finishedMessage(), {"OK"}, true);
        }
        
        printf("Done");
        inst::ui::loadMainMenu();
        inst::util::deinitInstallServices();
        return;
    }
}