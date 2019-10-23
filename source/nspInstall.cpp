#include <cstring>
#include <sstream>
#include "install/install_nsp.hpp"
#include "nx/fs.hpp"
#include "util/file_util.hpp"
#include "util/title_util.hpp"
#include "util/error.hpp"

#include "nspInstall.hpp"
#include "ui/MainApplication.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;

    void setNspInfoText(std::string ourText){
        mainApp->nspinstPage->pageInfoText->SetText(ourText);
        mainApp->CallForRender();
    }

    void loadMainMenu(){
        mainApp->LoadLayout(mainApp->mainPage);
    }
}

namespace nspInstStuff {
    FsStorageId m_destStorageId = FsStorageId_SdCard;

    bool installNspFromFile(std::string ourNsp, int whereToInstall)
    {
        std::vector<std::string> installList;
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
                tin::install::nsp::NSPInstallTask task(simpleFS, m_destStorageId, 1);

                printf("NSP_INSTALL_PREPARING\n");
                task.Prepare();
                printf("Pre Install Records: \n");
                //task.DebugPrintInstallData();

                //std::stringstream ss;
                //ss << translate(Translate::NSP_INSTALLING) << " " << tin::util::GetTitleName(task.GetTitleId(), task.GetContentMetaType()) << " (" << (i + 1) << "/" << installList.size() << ")";
                //manager.m_printConsole->flags |= CONSOLE_COLOR_BOLD;
                //tin::util::PrintTextCentred(ss.str());
                //manager.m_printConsole->flags &= ~CONSOLE_COLOR_BOLD;

                inst::ui::setNspInfoText("Installing " + ourNsp + "...");
                task.Begin();
                printf("Post Install Records: \n");
                //task.DebugPrintInstallData();
            }
            catch (std::exception& e)
            {
                inst::ui::setNspInfoText("Failed to install NSP");
                printf("NSP_INSTALL_FAILED\n");
                printf("Failed to install NSP");
                printf("%s", e.what());
                fprintf(stdout, "%s", e.what());
                return false;
            }
        }

        printf("Done");
        inst::ui::loadMainMenu();
        return true;
    }
}
