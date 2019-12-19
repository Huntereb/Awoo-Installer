#include <filesystem>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/instPage.hpp"
#include "ui/optionsPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/curl.hpp"
#include "sdInstall.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    optionsPage::optionsPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        if (std::filesystem::exists(inst::config::appDir + "/background.png")) this->SetBackgroundImage(inst::config::appDir + "/background.png");
        else this->SetBackgroundImage("romfs:/images/background.jpg");
        this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#170909FF"));
        this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#17090980"));
        this->botRect = Rectangle::New(0, 660, 1280, 60, COLOR("#17090980"));
        this->titleImage = Image::New(0, 0, "romfs:/images/logo.png");
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

    void optionsPage::checkForUpdate(std::vector<std::string> updateInfo) {
            if (!mainApp->CreateShowDialog("Update available", "Awoo Installer " + updateInfo[0] + " is available now! Ready to update?", {"Update", "Cancel"}, false)) {
                inst::ui::loadInstallScreen();
                inst::ui::setTopInstInfoText("Updating to Awoo Installer " + updateInfo[0]);
                inst::ui::setInstBarPerc(0);
                inst::ui::setInstInfoText("Downloading Awoo Installer " + updateInfo[0]);
                try {
                    romfsExit();
                    std::string curName = inst::config::appDir + "/Awoo-Installer.nro";
                    std::string downloadName = inst::config::appDir + "/temp_download";
                    inst::curl::downloadFile(updateInfo[1], downloadName.c_str(), 0, true);
                    if (std::filesystem::exists(curName)) std::filesystem::remove(curName);
                    std::filesystem::rename(downloadName, curName);
                    mainApp->CreateShowDialog("Update complete!", "The software will now be closed.", {"OK"}, false);
                } catch (...) {
                    mainApp->CreateShowDialog("Update failed!", "The software will now be closed.", {"OK"}, false);
                }
                mainApp->FadeOut();
                mainApp->Close();
            }
        return;
    }

    std::string optionsPage::getMenuOptionIcon(bool ourBool) {
        if(ourBool) return "romfs:/images/icons/check-box-outline.png";
        else return "romfs:/images/icons/checkbox-blank-outline.png";
    }

    void optionsPage::setMenuText() {
        this->menu->ClearItems();
        auto ignoreFirmOption = pu::ui::elm::MenuItem::New("Ignore minimum firmware version required by titles");
        ignoreFirmOption->SetColor(COLOR("#FFFFFFFF"));
        ignoreFirmOption->SetIcon(this->getMenuOptionIcon(inst::config::ignoreReqVers));
        this->menu->AddItem(ignoreFirmOption);
        auto validateOption = pu::ui::elm::MenuItem::New("Verify NCA signatures before installation");
        validateOption->SetColor(COLOR("#FFFFFFFF"));
        validateOption->SetIcon(this->getMenuOptionIcon(inst::config::validateNCAs));
        this->menu->AddItem(validateOption);
        auto overclockOption = pu::ui::elm::MenuItem::New("Enable \"boost mode\" during installations");
        overclockOption->SetColor(COLOR("#FFFFFFFF"));
        overclockOption->SetIcon(this->getMenuOptionIcon(inst::config::overClock));
        this->menu->AddItem(overclockOption);
        auto deletePromptOption = pu::ui::elm::MenuItem::New("Ask to delete original files after installation");
        deletePromptOption->SetColor(COLOR("#FFFFFFFF"));
        deletePromptOption->SetIcon(this->getMenuOptionIcon(inst::config::deletePrompt));
        this->menu->AddItem(deletePromptOption);
        auto autoUpdateOption = pu::ui::elm::MenuItem::New("Check for updates automatically");
        autoUpdateOption->SetColor(COLOR("#FFFFFFFF"));
        autoUpdateOption->SetIcon(this->getMenuOptionIcon(inst::config::autoUpdate));
        this->menu->AddItem(autoUpdateOption);
        auto gayModeOption = pu::ui::elm::MenuItem::New("Remove anime");
        gayModeOption->SetColor(COLOR("#FFFFFFFF"));
        gayModeOption->SetIcon(this->getMenuOptionIcon(inst::config::gayMode));
        this->menu->AddItem(gayModeOption);
        auto sigPatchesUrlOption = pu::ui::elm::MenuItem::New("Signature patches source URL: " + inst::util::shortenString(inst::config::sigPatchesUrl, 42, false));
        sigPatchesUrlOption->SetColor(COLOR("#FFFFFFFF"));
        this->menu->AddItem(sigPatchesUrlOption);
        auto updateOption = pu::ui::elm::MenuItem::New("Check for updates");
        updateOption->SetColor(COLOR("#FFFFFFFF"));
        this->menu->AddItem(updateOption);
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
            std::vector<std::string> downloadUrl;
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
                    inst::config::autoUpdate = !inst::config::autoUpdate;
                    inst::config::setConfig();
                    this->setMenuText();
                    break;
                case 5:
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
                case 6:
                    keyboardResult = inst::util::softwareKeyboard("Enter the URL to obtain Signature Patches from", inst::config::sigPatchesUrl.c_str(), 500);
                    if (keyboardResult.size() > 0) {
                        inst::config::sigPatchesUrl = keyboardResult;
                        inst::config::setConfig();
                        this->setMenuText();
                    }
                    break;
                case 7:
                    if (inst::util::getIPAddress() == "1.0.0.127") {
                        inst::ui::mainApp->CreateShowDialog("Network connection not available", "Check that airplane mode is disabled and you're connected to a local network.", {"OK"}, true);
                        break;
                    }
                    downloadUrl = inst::util::checkForAppUpdate();
                    if (!downloadUrl.size()) {
                        mainApp->CreateShowDialog("No updates found", "You are on the latest version of Awoo Installer!", {"OK"}, false);
                        break;
                    }
                    this->checkForUpdate(downloadUrl);
                    break;
                case 8:
                    inst::ui::mainApp->CreateShowDialog("Thanks to the following people!", "- HookedBehemoth for A LOT of contributions\n- Adubbz and other contributors for Tinfoil\n- XorTroll for Plutonium and Goldleaf\n- blawar (wife beater) and nicoboss for NSZ support\n- The kind folks at the AtlasNX Discuck (or at least some of them)\n- The also kind folks at the RetroNX Discuck (of no direct involvement)\n- namako8982 for the Momiji art\n- TheXzoron for being a baka", {"Close"}, true);
                    break;
                default:
                    break;
            }
        }
    }
}
