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
        inst::ui::setInstInfoText("waiting for usb...\n");

        while(true) {
            hidScanInput();
            if (hidKeysDown(CONTROLLER_P1_AUTO) & KEY_B)
                break;
            rc = usbDsWaitReady(1000000);
            if (R_SUCCEEDED(rc)) break;
            else if ((rc & 0x3FFFFF) != 0xEA01)
                return {};
        }

        inst::ui::setInstInfoText("USB install ready!\n");
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
        std::string nspExt = ".nsp";
        std::string nszExt = ".nsz";

        while (std::getline(nspNameStream, segment, '\n')) {
            if (segment.compare(segment.size() - nspExt.size(), nspExt.size(), nspExt) == 0 || segment.compare(segment.size() - nszExt.size(), nszExt.size(), nszExt) == 0)
                nspNames.push_back(segment);
        }

        return nspNames;
    }

    void installNspUsb(std::vector<std::string> ourNspList, int ourStorage) {
        inst::util::initInstallServices();

        inst::ui::loadInstallScreen();
        NcmStorageId m_destStorageId = NcmStorageId_SdCard;

        if (ourStorage) m_destStorageId = NcmStorageId_BuiltInUser;

        for (std::string nspName: ourNspList) {
            try {
                tin::install::nsp::USBNSP usbNSP(nspName);
                inst::ui::setInstInfoText("installing" + nspName);
                tin::install::nsp::RemoteNSPInstall install(m_destStorageId, inst::config::ignoreReqVers, &usbNSP);

                install.Prepare();
                install.Begin();
            } catch(...) {
                int rc = inst::ui::mainApp->CreateShowDialog("install failed", nspName + " failed\ndo you want to continue?", {"yes", "cancel"}, true);
                if (rc == 1)
                    break;
            }
        }

        tin::util::USBCmdManager::SendExitCmd();

        inst::ui::setInstInfoText("finished installation");
        inst::ui::mainApp->CreateShowDialog("Done", "Back to menu?", {"ok"}, true);
        inst::ui::loadMainMenu();
        inst::util::deinitInstallServices();        
    }
}