#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/nspInstPage.hpp"
#include "nspInstall.hpp"
#include "util.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;
    extern MainApplication *netinstPage;

    std::vector<std::filesystem::path> ourFiles;

    nspInstPage::nspInstPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->topText = TextBlock::New(10, 2, "Awoo Installer", 35);
        this->topText->SetColor(COLOR("#FFFFFFFF"));
        this->pageInfoText = TextBlock::New(10, 45, "", 35);
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
        nspInstStuff::OnIgnoreReqFirmVersionSelected(ourFiles[this->menu->GetSelectedIndex()].string().erase(0, 6));
        mainApp->CreateShowDialog(ourFiles[this->menu->GetSelectedIndex()].string().erase(0, 6) + " installed!", "", {"OK"}, true);
        this->pageInfoText->SetText("");
        mainApp->LoadLayout(mainApp->mainPage);
        return;
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
