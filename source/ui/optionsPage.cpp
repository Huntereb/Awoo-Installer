#include <filesystem>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/instPage.hpp"
#include "ui/optionsPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    std::vector<std::string> ourMenuEntries = {"Ignore minimum firmware version required by NSP files", "Validate NCA signature (brick protection)", "Ask to delete NSP files after installation", "Remove anime", "Signature patches source URL: "};

    optionsPage::optionsPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->SetBackgroundImage("romfs:/background.jpg");
        this->topRect = Rectangle::New(0, 0, 1280, 93, COLOR("#170909FF"));
        this->infoRect = Rectangle::New(0, 93, 1280, 60, COLOR("#17090980"));
        this->botRect = Rectangle::New(0, 660, 1280, 60, COLOR("#17090980"));
        this->titleImage = Image::New(0, 0, "romfs:/logo.png");
        this->appVersionText = TextBlock::New(480, 49, "v" + inst::config::appVersion, 22);
        this->appVersionText->SetColor(COLOR("#FFFFFFFF"));
        this->pageInfoText = TextBlock::New(10, 109, "Change Awoo Installer's settings!", 30);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->butText = TextBlock::New(10, 678, "\ue0e0 Select/Change    \ue0e1 Cancel ", 24);
        this->butText->SetColor(COLOR("#FFFFFFFF"));
        this->menu = pu::ui::elm::Menu::New(0, 154, 1280, COLOR("#FFFFFF00"), 84, (506 / 84));
        this->menu->SetOnFocusColor(COLOR("#00000033"));
        this->menu->SetScrollbarColor(COLOR("#17090980"));
        this->Add(this->topRect);
        this->Add(this->infoRect);
        this->Add(this->botRect);
        this->Add(this->titleImage);
        this->Add(this->appVersionText);
        this->Add(this->butText);
        this->Add(this->pageInfoText);
        optionsPage::setMenuText();
        this->Add(this->menu);
    }

    std::string optionsPage::getMenuOptionIcon(bool ourBool) {
        if(ourBool) return "romfs:/check-box-outline.png";
        else return "romfs:/checkbox-blank-outline.png";
    }

    void optionsPage::setMenuText() {
        this->menu->ClearItems();
        auto ignoreFirmOption = pu::ui::elm::MenuItem::New(ourMenuEntries[0]);
        ignoreFirmOption->SetColor(COLOR("#FFFFFFFF"));
        ignoreFirmOption->SetIcon(optionsPage::getMenuOptionIcon(inst::config::ignoreReqVers));
        this->menu->AddItem(ignoreFirmOption);
        auto validateOption = pu::ui::elm::MenuItem::New(ourMenuEntries[1]);
        validateOption->SetColor(COLOR("#FFFFFFFF"));
        validateOption->SetIcon(optionsPage::getMenuOptionIcon(inst::config::validateNCAs));
        this->menu->AddItem(validateOption);
        auto deletePromptOption = pu::ui::elm::MenuItem::New(ourMenuEntries[2]);
        deletePromptOption->SetColor(COLOR("#FFFFFFFF"));
        deletePromptOption->SetIcon(optionsPage::getMenuOptionIcon(inst::config::deletePrompt));
        this->menu->AddItem(deletePromptOption);
        auto gayModeOption = pu::ui::elm::MenuItem::New(ourMenuEntries[3]);
        gayModeOption->SetColor(COLOR("#FFFFFFFF"));
        gayModeOption->SetIcon(optionsPage::getMenuOptionIcon(inst::config::gayMode));
        this->menu->AddItem(gayModeOption);
        auto sigPatchesUrlOption = pu::ui::elm::MenuItem::New(ourMenuEntries[4] + inst::util::shortenString(inst::config::sigPatchesUrl, 42, false));
        sigPatchesUrlOption->SetColor(COLOR("#FFFFFFFF"));
        this->menu->AddItem(sigPatchesUrlOption);
        auto creditsOption = pu::ui::elm::MenuItem::New("Credits");
        creditsOption->SetColor(COLOR("#FFFFFFFF"));
        this->menu->AddItem(creditsOption);
        auto licenseOption = pu::ui::elm::MenuItem::New("Third Party Licenses");
        licenseOption->SetColor(COLOR("#FFFFFFFF"));
        this->menu->AddItem(licenseOption);
    }

    void optionsPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & KEY_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if ((Down & KEY_A) || (Up & KEY_TOUCH)) {
            Result rc=0;
            char tmpoutstr[128] = {0};
            switch (this->menu->GetSelectedIndex()) {
                case 0:
                    inst::config::ignoreReqVers = !inst::config::ignoreReqVers;
                    inst::config::setConfig();
                    optionsPage::setMenuText();
                    break;
                case 1:
                    inst::config::validateNCAs = !inst::config::validateNCAs;
                    inst::config::setConfig();
                    optionsPage::setMenuText();
                    break;
                case 2:
                    if (inst::config::deletePrompt) inst::config::deletePrompt = false;
                    else inst::config::deletePrompt = true;
                    inst::config::setConfig();
                    optionsPage::setMenuText();
                    break;
                case 3:
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
                case 4:
                    SwkbdConfig kbd;
                    rc = swkbdCreate(&kbd, 0);
                    if (R_SUCCEEDED(rc)) {
                        swkbdConfigMakePresetDefault(&kbd);
                        swkbdConfigSetGuideText(&kbd, "Enter the URL to obtain Signature Patches from");
                        swkbdConfigSetInitialText(&kbd, inst::config::sigPatchesUrl.c_str());
                        rc = swkbdShow(&kbd, tmpoutstr, sizeof(tmpoutstr));
                        swkbdClose(&kbd);
                        if (R_SUCCEEDED(rc) && tmpoutstr[0] != 0) {
                            inst::config::sigPatchesUrl = tmpoutstr;
                            inst::config::setConfig();
                            optionsPage::setMenuText();
                        }
                    }
                    break;
                case 5:
                    inst::ui::mainApp->CreateShowDialog("Thanks to the following people!", "- HookedBehemoth for screen-nx and his Plutonium fork\n- Adubbz and other contributors for Tinfoil\n- XorTroll for Plutonium and Goldleaf\n- blawar (wife beater) and nicoboss for NSZ support\n- The kind folks at the AtlasNX Discuck\n- The also kind folks at the RetroNX Discuck\n- namako8982 for the Momiji art\n- TheXzoron for being a baka", {"Close"}, true);
                    break;
                case 6:
                    inst::ui::mainApp->CreateShowDialog("Third Party Licenses", "Licenses to the libraries and software used in Awoo Installer are packaged with each release\nwithin the source code, or are distributed upon compilation of the software.\nPlease see the releases page for a copy of the source code and license information.", {"Close"}, true);
                    break;
                default:
                    break;
            }
        }
    }
}
