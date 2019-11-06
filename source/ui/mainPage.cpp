#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "sigInstall.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    MainPage::MainPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->SetBackgroundImage("romfs:/background.jpg");
        this->topRect = Rectangle::New(0, 0, 1280, 93, COLOR("#170909FF"));
        this->botRect = Rectangle::New(0, 660, 1280, 60, COLOR("#17090980"));
        this->titleImage = Image::New(0, 0, "romfs:/logo.png");
        this->butText = TextBlock::New(10, 678, "\ue0e0 Select    \ue0e1 Exit ", 24);
        this->butText->SetColor(COLOR("#FFFFFFFF"));
        this->optionMenu = pu::ui::elm::Menu::New(0, 93, 1280, COLOR("#67000000"), 113, (567 / 113));
        this->optionMenu->SetOnFocusColor(COLOR("#00000033"));
        this->optionMenu->SetScrollbarColor(COLOR("#170909FF"));
        this->installMenuItem = pu::ui::elm::MenuItem::New("Install NSP from SD Card");
        this->installMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->installMenuItem->SetIcon("romfs:/micro-sd.png");
        this->netInstallMenuItem = pu::ui::elm::MenuItem::New("Install NSP Over LAN or Internet");
        this->netInstallMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->netInstallMenuItem->SetIcon("romfs:/cloud-download.png");
        this->sigPatchesMenuItem = pu::ui::elm::MenuItem::New("Manage Signature Patches");
        this->sigPatchesMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->sigPatchesMenuItem->SetIcon("romfs:/wrench.png");
        this->settingsMenuItem = pu::ui::elm::MenuItem::New("Settings");
        this->settingsMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->settingsMenuItem->SetIcon("romfs:/settings.png");
        this->exitMenuItem = pu::ui::elm::MenuItem::New("Exit");
        this->exitMenuItem->SetColor(COLOR("#FFFFFFFF"));
        this->exitMenuItem->SetIcon("romfs:/exit-run.png");
        this->awooImage = Image::New(410, 190, "romfs:/awoos/5bbdbcf9a5625cd307c9e9bc360d78bd.png");
        this->eggImage = Image::New(410, 190, "romfs:/awoos/a8cb40e465dadaf9708c9b1896777ce6.png");
        this->Add(this->topRect);
        this->Add(this->botRect);
        this->Add(this->titleImage);
        this->Add(this->butText);
        this->optionMenu->AddItem(this->installMenuItem);
        this->optionMenu->AddItem(this->netInstallMenuItem);
        this->optionMenu->AddItem(this->sigPatchesMenuItem);
        this->optionMenu->AddItem(this->settingsMenuItem);
        this->optionMenu->AddItem(this->exitMenuItem);
        this->Add(this->optionMenu);
        this->Add(this->awooImage);
        this->Add(this->eggImage);
        if (inst::config::gayMode) this->awooImage->SetVisible(false);
        this->eggImage->SetVisible(false);
    }

    void MainPage::installMenuItem_Click() {
        if (inst::util::getDirectoryFiles("sdmc:/", {".nsp", ".nsz"}).size()) {
            mainApp->nspinstPage->drawMenuItems(true);
            mainApp->LoadLayout(mainApp->nspinstPage);
        } else {
            mainApp->CreateShowDialog("No NSP or NSZ files found!", "Copy them to the root of your SD card!", {"OK"}, true);
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
        if ((Down & KEY_A) || (Up & KEY_TOUCH)) {
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
        if (Down & KEY_X) {
            this->awooImage->SetVisible(false);
            this->eggImage->SetVisible(true);
        }
        if (Up & KEY_X) {
            this->eggImage->SetVisible(false);
            if (!inst::config::gayMode) this->awooImage->SetVisible(true);
        }
    }
}
