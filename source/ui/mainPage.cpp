#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "util/util.hpp"
#include "sigInstall.hpp"

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
        this->installMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->netInstallMenuItem = pu::ui::elm::MenuItem::New("Install NSP Over LAN or Internet");
        this->netInstallMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->sigPatchesMenuItem = pu::ui::elm::MenuItem::New("Manage Signature Patches");
        this->sigPatchesMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->settingsMenuItem = pu::ui::elm::MenuItem::New("Settings");
        this->settingsMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->exitMenuItem = pu::ui::elm::MenuItem::New("Exit");
        this->exitMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->Add(this->topText);
        this->optionMenu->AddItem(this->installMenuItem);
        this->optionMenu->AddItem(this->netInstallMenuItem);
        this->optionMenu->AddItem(this->sigPatchesMenuItem);
        this->optionMenu->AddItem(this->settingsMenuItem);
        this->optionMenu->AddItem(this->exitMenuItem);
        this->Add(this->optionMenu);
    }

    void MainPage::installMenuItem_Click() {
        if (inst::util::getDirectoryFiles("sdmc:/", {".nsp"}).size()) {
            mainApp->LoadLayout(mainApp->nspinstPage);
        } else {
            mainApp->CreateShowDialog("No NSP files found!", "NSPs can be placed on the root of your SD card!", {"OK"}, true);
        }
    }

    void MainPage::netInstallMenuItem_Click() {
        mainApp->netinstPage->startNetwork();
    }

    void MainPage::sigPatchesMenuItem_Click() {
        sig::installSigPatches();
    }

    void MainPage::exitMenuItem_Click() {
        mainApp->Close();
    }

    void MainPage::settingsMenuItem_Click() {
        mainApp->LoadLayout(mainApp->optionspage);
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
                    MainPage::settingsMenuItem_Click();
                    break;
                case 4:
                    MainPage::exitMenuItem_Click();
                    break;
                default:
                    break;
            }
        }
    }
}
