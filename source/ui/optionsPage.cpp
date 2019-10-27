#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/instPage.hpp"
#include "ui/optionsPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    std::vector<std::string> ourMenuEntries = {"Ignore Required Firmware Version - ", "Remove Anime - "};

    optionsPage::optionsPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->SetBackgroundImage("romfs:/background.jpg");
        this->topRect = Rectangle::New(0, 0, 1280, 93, COLOR("#170909FF"));
        this->infoRect = Rectangle::New(0, 93, 1280, 60, COLOR("#17090980"));
        this->botRect = Rectangle::New(0, 660, 1280, 60, COLOR("#17090980"));
        this->titleImage = Image::New(0, 0, "romfs:/logo.png");
        this->pageInfoText = TextBlock::New(10, 109, "Change application settings!", 30);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->butText = TextBlock::New(10, 676, "(A)-Toggle (B)-Cancel", 30);
        this->butText->SetColor(COLOR("#FFFFFFFF"));
        this->menu = pu::ui::elm::Menu::New(0, 153, 1280, COLOR("#FFFFFF00"), 84, (506 / 84));
        this->menu->SetOnFocusColor(COLOR("#00000033"));
        this->menu->SetScrollbarColor(COLOR("#17090980"));
        this->Add(this->topRect);
        this->Add(this->infoRect);
        this->Add(this->botRect);
        this->Add(this->titleImage);
        this->Add(this->butText);
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
                    if (inst::config::gayMode) {
                        inst::config::gayMode = false;
                        mainApp->mainPage->awooImage->SetVisible(true);
                        mainApp->instpage->awooImage->SetVisible(true);
                    }
                    else {
                        inst::config::gayMode = true;
                        mainApp->mainPage->awooImage->SetVisible(false);
                        mainApp->instpage->awooImage->SetVisible(false);
                    }
                    inst::config::setConfig();
                    optionsPage::setMenuText();
                    break;
                default:
                    break;
            }
        }
    }
}
