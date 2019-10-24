#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/nspInstPage.hpp"
#include "nspInstall.hpp"
#include "util.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    std::vector<std::filesystem::path> ourFiles;

    nspInstPage::nspInstPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->topText = TextBlock::New(10, 2, "Awoo Installer", 35);
        this->topText->SetColor(COLOR("#FFFFFFFF"));
        this->pageInfoText = TextBlock::New(10, 45, "Select a NSP to install! Put NSP files on the root of your SD!", 35);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->menu = pu::ui::elm::Menu::New(0, 160, 1280, COLOR("#FFFFFF00"), 80, (560 / 80));
        this->menu->SetOnFocusColor(COLOR("#00000033"));
        ourFiles = util::getDirectoryFiles("sdmc:/", {".nsp"});
        for (auto& file: ourFiles) {
            pu::String itm = file.string().erase(0, 6);
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(COLOR("#FFFFFFFF"));
            this->menu->AddItem(ourEntry);
        }
        this->Add(this->topText);
        this->Add(this->pageInfoText);
        this->Add(this->menu);
    }

    void nspInstPage::startInstall() {
        std::string ourNsp = ourFiles[this->menu->GetSelectedIndex()].string().erase(0, 6);
        int dialogResult = mainApp->CreateShowDialog("Where should " + ourNsp + " be installed to?", "Press B to cancel", {"SD", "Internal Storage"}, false);
        if (dialogResult == -1) return;
        nspInstStuff::installNspFromFile(ourNsp, dialogResult);
    }

    void nspInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & KEY_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if (Down & KEY_A) {
            nspInstPage::startInstall();
        }
    }
}
