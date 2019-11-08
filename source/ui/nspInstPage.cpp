#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/nspInstPage.hpp"
#include "nspInstall.hpp"
#include "util/util.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    std::vector<std::filesystem::path> nspInstPage::ourFiles;
    std::vector<std::filesystem::path> nspInstPage::selectedNsps;

    nspInstPage::nspInstPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->SetBackgroundImage("romfs:/background.jpg");
        this->topRect = Rectangle::New(0, 0, 1280, 93, COLOR("#170909FF"));
        this->infoRect = Rectangle::New(0, 93, 1280, 60, COLOR("#17090980"));
        this->botRect = Rectangle::New(0, 660, 1280, 60, COLOR("#17090980"));
        this->titleImage = Image::New(0, 0, "romfs:/logo.png");
        this->pageInfoText = TextBlock::New(10, 109, "Select NSP files to install, then press the Plus button!", 30);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->butText = TextBlock::New(10, 678, "\ue0e0 Select NSP    \ue0e3 Select All    \ue0ef Install NSP(s)    \ue0e2 Help    \ue0e1 Cancel ", 24);
        this->butText->SetColor(COLOR("#FFFFFFFF"));
        this->menu = pu::ui::elm::Menu::New(0, 154, 1280, COLOR("#FFFFFF00"), 84, (506 / 84));
        this->menu->SetOnFocusColor(COLOR("#00000033"));
        this->menu->SetScrollbarColor(COLOR("#17090980"));
        this->Add(this->topRect);
        this->Add(this->infoRect);
        this->Add(this->botRect);
        this->Add(this->titleImage);
        this->Add(this->butText);
        this->Add(this->pageInfoText);
        this->Add(this->menu);
    }

    void nspInstPage::drawMenuItems(bool clearItems) {
        if (clearItems) nspInstPage::selectedNsps = {};
        this->menu->ClearItems();
        nspInstPage::ourFiles = util::getDirectoryFiles("sdmc:/", {".nsp", ".nsz"});
        for (auto& file: nspInstPage::ourFiles) {
            pu::String itm = inst::util::shortenString(file.string().erase(0, 6), 64, true);
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(COLOR("#FFFFFFFF"));
            ourEntry->SetIcon("romfs:/checkbox-blank-outline.png");
            for (long unsigned int i = 0; i < nspInstPage::selectedNsps.size(); i++) {
                if (nspInstPage::selectedNsps[i] == file) {
                    ourEntry->SetIcon("romfs:/check-box-outline.png");
                }
            }
            this->menu->AddItem(ourEntry);
        }
    }

    void nspInstPage::selectNsp(int selectedIndex) {
        if (this->menu->GetItems()[selectedIndex]->GetIcon() == "romfs:/check-box-outline.png") {
            for (long unsigned int i = 0; i < nspInstPage::selectedNsps.size(); i++) {
                if (nspInstPage::selectedNsps[i] == nspInstPage::ourFiles[selectedIndex].string()) nspInstPage::selectedNsps.erase(nspInstPage::selectedNsps.begin() + i);
            }
        } else nspInstPage::selectedNsps.push_back(nspInstPage::ourFiles[selectedIndex]);
        nspInstPage::drawMenuItems(false);
    }

    void nspInstPage::startInstall() {
        int dialogResult = -1;
        if (nspInstPage::selectedNsps.size() == 1) {
            std::string ourNsp = nspInstPage::selectedNsps[0].string().erase(0, 6);
            dialogResult = mainApp->CreateShowDialog("Where should " + inst::util::shortenString(ourNsp, 48, true) + " be installed to?", "Press B to cancel", {"SD Card", "Internal Storage"}, false);
        } else dialogResult = mainApp->CreateShowDialog("Where should the selected " + std::to_string(nspInstPage::selectedNsps.size()) + " files be installed to?", "Press B to cancel", {"SD Card", "Internal Storage"}, false);
        if (dialogResult == -1) return;
        nspInstStuff::installNspFromFile(nspInstPage::selectedNsps, dialogResult);
    }

    void nspInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & KEY_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if ((Down & KEY_A) || (Up & KEY_TOUCH)) {
            nspInstPage::selectNsp(this->menu->GetSelectedIndex());
            if (this->menu->GetItems().size() == 1) {
                nspInstPage::startInstall();
                nspInstPage::selectNsp(this->menu->GetSelectedIndex());
            }
        }
        if ((Down & KEY_Y)) {
            if (nspInstPage::selectedNsps.size() == this->menu->GetItems().size()) nspInstPage::drawMenuItems(true);
            else {
                for (long unsigned int i = 0; i < this->menu->GetItems().size(); i++) {
                    if (this->menu->GetItems()[i]->GetIcon() == "romfs:/check-box-outline.png") continue;
                    else nspInstPage::selectNsp(i);
                }
                nspInstPage::drawMenuItems(false);
            }
        }
        if ((Down & KEY_X)) {
            inst::ui::mainApp->CreateShowDialog("Help", "Copy your NSP or NSZ files to the root (top) of your SD card, select the ones\nyou want to install, then press the Plus button.", {"OK"}, true);
        }
        if (Down & KEY_PLUS) {
            if (nspInstPage::selectedNsps.size() == 0) {
                nspInstPage::selectNsp(this->menu->GetSelectedIndex());
                nspInstPage::startInstall();
                return;
            }
            nspInstPage::startInstall();
        }
    }
}
