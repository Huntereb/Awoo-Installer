#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/nspInstPage.hpp"
#include "nspInstall.hpp"
#include "util/util.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    std::vector<std::filesystem::path> ourFiles;

    nspInstPage::nspInstPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->SetBackgroundImage("romfs:/background.jpg");
        this->topRect = Rectangle::New(0, 0, 1280, 93, COLOR("#170909FF"));
        this->infoRect = Rectangle::New(0, 93, 1280, 60, COLOR("#17090980"));
        this->botRect = Rectangle::New(0, 660, 1280, 60, COLOR("#17090980"));
        this->titleImage = Image::New(0, 0, "romfs:/logo.png");
        this->pageInfoText = TextBlock::New(10, 109, "Select a NSP to install! Put NSP files on the root of your SD!", 30);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->butText = TextBlock::New(10, 676, "(A)-Install NSP (B)-Cancel", 30);
        this->butText->SetColor(COLOR("#FFFFFFFF"));
        this->menu = pu::ui::elm::Menu::New(0, 153, 1280, COLOR("#FFFFFF00"), 84, (506 / 84));
        this->menu->SetOnFocusColor(COLOR("#00000033"));
        this->menu->SetScrollbarColor(COLOR("#17090980"));
        ourFiles = util::getDirectoryFiles("sdmc:/", {".nsp"});
        for (auto& file: ourFiles) {
            pu::String itm = inst::util::shortenString(file.string().erase(0, 6), 64, true);
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(COLOR("#FFFFFFFF"));
            ourEntry->SetIcon("romfs:/package.png");
            this->menu->AddItem(ourEntry);
        }
        this->Add(this->topRect);
        this->Add(this->infoRect);
        this->Add(this->botRect);
        this->Add(this->titleImage);
        this->Add(this->butText);
        this->Add(this->pageInfoText);
        this->Add(this->menu);
    }

    void nspInstPage::startInstall() {
        std::string ourNsp = ourFiles[this->menu->GetSelectedIndex()].string().erase(0, 6);
        int dialogResult = mainApp->CreateShowDialog("Where should " + inst::util::shortenString(ourNsp, 64, true) + " be installed to?", "Press B to cancel", {"SD Card", "Internal Storage"}, false);
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
