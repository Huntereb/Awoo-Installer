#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/optionsPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    std::vector<std::string> ourMenuEntries = {"Ignore Required Firmware Version - ", "Disable Anime - "};

    optionsPage::optionsPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->topText = TextBlock::New(10, 2, "Awoo Installer", 35);
        this->topText->SetColor(COLOR("#FFFFFFFF"));
        this->pageInfoText = TextBlock::New(10, 45, "Settings Menu", 35);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->menu = pu::ui::elm::Menu::New(0, 160, 1280, COLOR("#FFFFFF00"), 80, (560 / 80));
        this->menu->SetOnFocusColor(COLOR("#00000033"));
        this->Add(this->topText);
        this->Add(this->pageInfoText);
        optionsPage::setMenuText();
        this->Add(this->menu);
    }

    std::string optionsPage::getMenuOptionText(bool ourBool) {
        if(ourBool) return "Enabled";
        else return "Disabled";
    }

    void optionsPage::setMenuText() {
        this->menu->ClearItems();
        auto ignoreFirmOption = pu::ui::elm::MenuItem::New(ourMenuEntries[0] + getMenuOptionText(inst::config::ignoreReqVers));
        ignoreFirmOption->SetColor(COLOR("#FFFFFFFF"));
        this->menu->AddItem(ignoreFirmOption);
        auto gayModeOption = pu::ui::elm::MenuItem::New(ourMenuEntries[1] + getMenuOptionText(inst::config::gayMode));
        gayModeOption->SetColor(COLOR("#FFFFFFFF"));
        this->menu->AddItem(gayModeOption);
    }

    void optionsPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & KEY_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if (Down & KEY_A) {
            switch (this->menu->GetSelectedIndex()) {
                case 0:
                    if (inst::config::ignoreReqVers) inst::config::ignoreReqVers = false;
                    else inst::config::ignoreReqVers = true;
                    inst::config::setConfig();
                    optionsPage::setMenuText();
                    break;
                case 1:
                    if (inst::config::gayMode) inst::config::gayMode = false;
                    else inst::config::gayMode = true;
                    inst::config::setConfig();
                    optionsPage::setMenuText();
                    break;
                default:
                    break;
            }
        }
    }
}
