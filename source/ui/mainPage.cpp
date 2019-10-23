#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "curl.hpp"
#include "util.hpp"
#include "unzip.hpp"
#include "netInstall.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    MainPage::MainPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->topText = TextBlock::New(10, 2, "Awoo Installer", 35);
        this->topText->SetColor(COLOR("#FFFFFFFF"));
        this->optionMenu = pu::ui::elm::Menu::New(0, 160, 1280, COLOR("#FFFFFF00"), 80, (560 / 80));
        this->optionMenu->SetOnFocusColor(COLOR("#00000033"));
        this->installMenuItem = pu::ui::elm::MenuItem::New("Install NSP");
        //this->installMenuItem->AddOnClick(std::bind(&MainPage::installMenuItem_Click, this));
        this->installMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->netInstallMenuItem = pu::ui::elm::MenuItem::New("Install NSP Over LAN");
        //this->netInstallMenuItem->AddOnClick(std::bind(&MainPage::netInstallMenuItem_Click, this));
        this->netInstallMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->sigPatchesMenuItem = pu::ui::elm::MenuItem::New("Install Signature Patches");
        //this->sigPatchesMenuItem->AddOnClick(std::bind(&MainPage::sigPatchesMenuItem_Click, this));
        this->sigPatchesMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->exitMenuItem = pu::ui::elm::MenuItem::New("Exit");
        //this->exitMenuItem->AddOnClick(std::bind(&MainPage::exitMenuItem_Click, this));
        this->exitMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->Add(this->topText);
        this->optionMenu->AddItem(this->installMenuItem);
        this->optionMenu->AddItem(this->netInstallMenuItem);
        this->optionMenu->AddItem(this->sigPatchesMenuItem);
        this->optionMenu->AddItem(this->exitMenuItem);
        this->Add(this->optionMenu);
    }

    void MainPage::installMenuItem_Click() {
        if (util::getDirectoryFiles("sdmc:/", {".nsp"}).size()) {
            mainApp->LoadLayout(mainApp->nspinstPage);
        } else {
            mainApp->CreateShowDialog("No NSP files found!", "NSPs can be placed on the root of your SD card!", {"OK"}, true);
        }
    }

    void MainPage::netInstallMenuItem_Click() {
        mainApp->netinstPage->startNetwork();
    }

    void MainPage::sigPatchesMenuItem_Click() {
        std::string ourPath = appVariables::appDir + "patches.zip";
        bool didDownload = curlStuff::downloadFile("http://github.com/Joonie86/hekate/releases/download/5.0.0J/Kosmos_patches_10_09_2019.zip", ourPath.c_str());
        bool didExtract = false;
        if (didDownload) didExtract = zipStuff::extractFile(ourPath, "sdmc:/");
        else {
            mainApp->CreateShowDialog("Could not download signature patches!", "", {"OK"}, true);
            return;
        }
        std::filesystem::remove(ourPath);
        if (didExtract) mainApp->CreateShowDialog("Install complete! Restart your console to apply!", "", {"OK"}, true);
        else {
            mainApp->CreateShowDialog("Could not extract files!", "", {"OK"}, true);
            return;
        }
        return;
    }

    void MainPage::exitMenuItem_Click() {
        mainApp->Close();
    }

    void MainPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if ((Down & KEY_PLUS) || (Down & KEY_MINUS) || (Down & KEY_B)) {
            mainApp->Close();
        }
        if (Down & KEY_A) {
            switch (this->optionMenu->GetSelectedIndex()) {
                case 0:
                    MainPage::installMenuItem_Click();
                    break;
                case 1:
                    MainPage::netInstallMenuItem_Click();
                    break;
                case 2:
                    MainPage::sigPatchesMenuItem_Click();
                    break;
                case 3:
                    MainPage::exitMenuItem_Click();
                    break;
                default:
                    break;
            }
        }
    }
}
