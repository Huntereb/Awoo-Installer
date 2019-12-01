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

    std::vector<std::string> ourMenuEntries = {"Ignore minimum firmware version required by titles", "Verify NCA signatures before installation", "Enable \"boost mode\" during installations", "Ask to delete original files after installation", "Remove anime", "Signature patches source URL: "};

    optionsPage::optionsPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        if (std::filesystem::exists(inst::config::appDir + "/background.png")) this->SetBackgroundImage(inst::config::appDir + "/background.png");
        else this->SetBackgroundImage("romfs:/background.jpg");
        this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#170909FF"));
        this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#17090980"));
        this->botRect = Rectangle::New(0, 660, 1280, 60, COLOR("#17090980"));
        this->titleImage = Image::New(0, 0, "romfs:/logo.png");
        this->appVersionText = TextBlock::New(480, 49, "v" + inst::config::appVersion, 22);
        this->appVersionText->SetColor(COLOR("#FFFFFFFF"));
        this->pageInfoText = TextBlock::New(10, 109, "Change Awoo Installer's settings!", 30);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->butText = TextBlock::New(10, 678, "\ue0e0 Select/Change    \ue0e1 Cancel ", 24);
        this->butText->SetColor(COLOR("#FFFFFFFF"));
        this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR("#FFFFFF00"), 84, (506 / 84));
        this->menu->SetOnFocusColor(COLOR("#00000033"));
        this->menu->SetScrollbarColor(COLOR("#17090980"));
        this->Add(this->topRect);
        this->Add(this->infoRect);
        this->Add(this->botRect);
        this->Add(this->titleImage);
        this->Add(this->appVersionText);
        this->Add(this->butText);
        this->Add(this->pageInfoText);
        this->setMenuText();
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
        ignoreFirmOption->SetIcon(this->getMenuOptionIcon(inst::config::ignoreReqVers));
        this->menu->AddItem(ignoreFirmOption);
        auto validateOption = pu::ui::elm::MenuItem::New(ourMenuEntries[1]);
        validateOption->SetColor(COLOR("#FFFFFFFF"));
        validateOption->SetIcon(this->getMenuOptionIcon(inst::config::validateNCAs));
        this->menu->AddItem(validateOption);
        auto overclockOption = pu::ui::elm::MenuItem::New(ourMenuEntries[2]);
        overclockOption->SetColor(COLOR("#FFFFFFFF"));
        overclockOption->SetIcon(this->getMenuOptionIcon(inst::config::overClock));
        this->menu->AddItem(overclockOption);
        auto deletePromptOption = pu::ui::elm::MenuItem::New(ourMenuEntries[3]);
        deletePromptOption->SetColor(COLOR("#FFFFFFFF"));
        deletePromptOption->SetIcon(this->getMenuOptionIcon(inst::config::deletePrompt));
        this->menu->AddItem(deletePromptOption);
        auto gayModeOption = pu::ui::elm::MenuItem::New(ourMenuEntries[4]);
        gayModeOption->SetColor(COLOR("#FFFFFFFF"));
        gayModeOption->SetIcon(this->getMenuOptionIcon(inst::config::gayMode));
        this->menu->AddItem(gayModeOption);
        auto sigPatchesUrlOption = pu::ui::elm::MenuItem::New(ourMenuEntries[5] + inst::util::shortenString(inst::config::sigPatchesUrl, 42, false));
        sigPatchesUrlOption->SetColor(COLOR("#FFFFFFFF"));
        this->menu->AddItem(sigPatchesUrlOption);
        auto creditsOption = pu::ui::elm::MenuItem::New("Credits");
        creditsOption->SetColor(COLOR("#FFFFFFFF"));
        this->menu->AddItem(creditsOption);
    }

    void optionsPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & KEY_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if ((Down & KEY_A) || (Up & KEY_TOUCH)) {
            std::string keyboardResult;
            switch (this->menu->GetSelectedIndex()) {
                case 0:
                    inst::config::ignoreReqVers = !inst::config::ignoreReqVers;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 1:
                    if (inst::config::validateNCAs) {
                        if (inst::ui::mainApp->CreateShowDialog("Warning!", "Some installable files may contain malicious contents! Only disable this\nfeature if you are absolutely certain the software you will be installing\nis trustworthy!\n\nDo you still want to disable NCA signature verification?", {"Cancel", "Yes, I want a brick"}, false) == 1) inst::config::validateNCAs = false;
                    } else inst::config::validateNCAs = true;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 2:
                    inst::config::overClock = !inst::config::overClock;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 3:
                    inst::config::deletePrompt = !inst::config::deletePrompt;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 4:
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
                    this->setMenuText();
                    break;
                case 5:
                    keyboardResult = inst::util::softwareKeyboard("Enter the URL to obtain Signature Patches from", inst::config::sigPatchesUrl.c_str(), 500);
                    if (keyboardResult.size() > 0) {
                        inst::config::sigPatchesUrl = keyboardResult;
                        inst::config::setConfig();
                        this->setMenuText();
                    }
                    break;
                case 6:
                    inst::ui::mainApp->CreateShowDialog("Thanks to the following people!", "- HookedBehemoth for A LOT of contributions\n- Adubbz and other contributors for Tinfoil\n- XorTroll for Plutonium and Goldleaf\n- blawar (wife beater) and nicoboss for NSZ support\n- The kind folks at the AtlasNX Discuck (or at least some of them)\n- The also kind folks at the RetroNX Discuck\n- namako8982 for the Momiji art\n- TheXzoron for being a baka", {"Close"}, true);
                    break;
                default:
                    break;
            }
        }
    }
}
