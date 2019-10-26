#include <cstring>
#include <sstream>
#include <filesystem>
#include "install/install_nsp.hpp"
#include "nx/fs.hpp"
#include "util/file_util.hpp"
#include "util/title_util.hpp"
#include "util/error.hpp"

#include "nspInstall.hpp"
#include "ui/MainApplication.hpp"
#include "config.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;

    void setInstInfoText(std::string ourText){
        mainApp->instpage->pageInfoText->SetText(ourText);
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
        mainApp->instpage->installBar->SetProgress(0);
        mainApp->instpage->installBar->SetVisible(false);
        mainApp->LoadLayout(mainApp->instpage);
        mainApp->CallForRender();
    }
}

namespace nspInstStuff {
    FsStorageId m_destStorageId = FsStorageId_SdCard;

    void installNspFromFile(std::string ourNsp, int whereToInstall)
    {
        appletLockExit();
        inst::ui::loadInstallScreen();
        std::vector<std::string> installList;
        bool nspInstalled = false;
        installList.push_back(ourNsp);

        if (whereToInstall) m_destStorageId = FsStorageId_NandUser;

        for (unsigned int i = 0; i < installList.size(); i++)
        {
            std::string path = "@Sdcard://" + installList[i];

            try
            {
                nx::fs::IFileSystem fileSystem;
                fileSystem.OpenFileSystemWithId(path, FsFileSystemType_ApplicationPackage, 0);
                tin::install::nsp::SimpleFileSystem simpleFS(fileSystem, "/", path + "/");
                //last arg is ignore required firm version, read from config for this
                tin::install::nsp::NSPInstallTask task(simpleFS, m_destStorageId, inst::config::ignoreReqVers);

                printf("NSP_INSTALL_PREPARING\n");
                inst::ui::setInstInfoText("Preparing installation...");
                task.Prepare();
                printf("Pre Install Records: \n");
                //task.DebugPrintInstallData();

                //std::stringstream ss;
                //ss << translate(Translate::NSP_INSTALLING) << " " << tin::util::GetTitleName(task.GetTitleId(), task.GetContentMetaType()) << " (" << (i + 1) << "/" << installList.size() << ")";
                //manager.m_printConsole->flags |= CONSOLE_COLOR_BOLD;
                //tin::util::PrintTextCentred(ss.str());
                //manager.m_printConsole->flags &= ~CONSOLE_COLOR_BOLD;

                task.Begin();
                printf("Post Install Records: \n");
                //task.DebugPrintInstallData();
                nspInstalled = true;
            }
            catch (std::exception& e)
            {
                printf("NSP_INSTALL_FAILED\n");
                printf("Failed to install NSP");
                printf("%s", e.what());
                fprintf(stdout, "%s", e.what());
                inst::ui::mainApp->CreateShowDialog("Failed to install NSP!", (std::string)e.what(), {"OK"}, true);
            }
        }

        if(nspInstalled) if(inst::ui::mainApp->CreateShowDialog(ourNsp + " installed! Delete NSP from SD card?", "", {"No","Yes"}, false)) std::filesystem::remove("sdmc:/" + ourNsp);

        printf("Done");
        appletUnlockExit();
        inst::ui::loadMainMenu();
        return;
    }
}
