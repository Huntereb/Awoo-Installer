#include <filesystem>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/netInstPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/curl.hpp"
#include "util/lang.hpp"
#include "netInstall.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    std::string lastFileID = "";
    std::string sourceString = "";

    netInstPage::netInstPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        if (std::filesystem::exists(inst::config::appDir + "/background.png")) this->SetBackgroundImage(inst::config::appDir + "/background.png");
        else this->SetBackgroundImage("romfs:/images/background.jpg");
        this->topRect = Rectangle::New(0, 0, 1280, 94, COLOR("#170909FF"));
        this->infoRect = Rectangle::New(0, 95, 1280, 60, COLOR("#17090980"));
        this->botRect = Rectangle::New(0, 660, 1280, 60, COLOR("#17090980"));
        if (inst::config::gayMode) {
            this->titleImage = Image::New(-113, 0, "romfs:/images/logo.png");
            this->appVersionText = TextBlock::New(367, 49, "v" + inst::config::appVersion, 22);
        }
        else {
            this->titleImage = Image::New(0, 0, "romfs:/images/logo.png");
            this->appVersionText = TextBlock::New(480, 49, "v" + inst::config::appVersion, 22);
        }
        this->appVersionText->SetColor(COLOR("#FFFFFFFF"));
        this->pageInfoText = TextBlock::New(10, 109, "", 30);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->butText = TextBlock::New(10, 678, "", 24);
        this->butText->SetColor(COLOR("#FFFFFFFF"));
        this->menu = pu::ui::elm::Menu::New(0, 156, 1280, COLOR("#FFFFFF00"), 84, (506 / 84));
        this->menu->SetOnFocusColor(COLOR("#00000033"));
        this->menu->SetScrollbarColor(COLOR("#17090980"));
        this->infoImage = Image::New(453, 292, "romfs:/images/icons/lan-connection-waiting.png");
        this->Add(this->topRect);
        this->Add(this->infoRect);
        this->Add(this->botRect);
        this->Add(this->titleImage);
        this->Add(this->appVersionText);
        this->Add(this->butText);
        this->Add(this->pageInfoText);
        this->Add(this->menu);
        this->Add(this->infoImage);
    }

    void netInstPage::drawMenuItems(bool clearItems) {
        if (clearItems) this->selectedUrls = {};
        if (clearItems) this->alternativeNames = {};
        this->menu->ClearItems();
        for (auto& url: this->ourUrls) {
            std::string itm = inst::util::shortenString(inst::util::formatUrlString(url), 56, true);
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(COLOR("#FFFFFFFF"));
            ourEntry->SetIcon("romfs:/images/icons/checkbox-blank-outline.png");
            for (long unsigned int i = 0; i < this->selectedUrls.size(); i++) {
                if (this->selectedUrls[i] == url) {
                    ourEntry->SetIcon("romfs:/images/icons/check-box-outline.png");
                }
            }
            this->menu->AddItem(ourEntry);
        }
    }

    void netInstPage::selectTitle(int selectedIndex) {
        if (this->menu->GetItems()[selectedIndex]->GetIcon() == "romfs:/images/icons/check-box-outline.png") {
            for (long unsigned int i = 0; i < this->selectedUrls.size(); i++) {
                if (this->selectedUrls[i] == this->ourUrls[selectedIndex]) this->selectedUrls.erase(this->selectedUrls.begin() + i);
            }
        } else this->selectedUrls.push_back(this->ourUrls[selectedIndex]);
        this->drawMenuItems(false);
    }

    void netInstPage::startNetwork() {
        this->butText->SetText("inst.net.buttons"_lang);
        this->menu->SetVisible(false);
        this->menu->ClearItems();
        this->infoImage->SetVisible(true);
        mainApp->LoadLayout(mainApp->netinstPage);
        this->ourUrls = netInstStuff::OnSelected();
        if (!this->ourUrls.size()) {
            mainApp->LoadLayout(mainApp->mainPage);
            return;
        } else if (this->ourUrls[0] == "supplyUrl") {
            std::string keyboardResult;
            switch (mainApp->CreateShowDialog("inst.net.src.title"_lang, "common.cancel_desc"_lang, {"inst.net.src.opt0"_lang, "inst.net.src.opt1"_lang}, false)) {
                case 0:
                    keyboardResult = inst::util::softwareKeyboard("inst.net.url.hint"_lang, inst::config::lastNetUrl, 500);
                    if (keyboardResult.size() > 0) {
                        if (inst::util::formatUrlString(keyboardResult) == "" || keyboardResult == "https://" || keyboardResult == "http://") {
                            mainApp->CreateShowDialog("inst.net.url.invalid"_lang, "", {"common.ok"_lang}, false);
                            break;
                        }
                        inst::config::lastNetUrl = keyboardResult;
                        inst::config::setConfig();
                        sourceString = "inst.net.url.source_string"_lang;
                        this->selectedUrls = {keyboardResult};
                        this->startInstall(true);
                        return;
                    }
                    break;
                case 1:
                    keyboardResult = inst::util::softwareKeyboard("inst.net.gdrive.hint"_lang, lastFileID, 50);
                    if (keyboardResult.size() > 0) {
                        lastFileID = keyboardResult;
                        std::string fileName = inst::util::getDriveFileName(keyboardResult);
                        if (fileName.size() > 0) this->alternativeNames = {fileName};
                        else this->alternativeNames = {"inst.net.gdrive.alt_name"_lang};
                        sourceString = "inst.net.gdrive.source_string"_lang;
                        this->selectedUrls = {"https://www.googleapis.com/drive/v3/files/" + keyboardResult + "?key=" + inst::config::gAuthKey + "&alt=media"};
                        this->startInstall(true);
                        return;
                    }
                    break;
            }
            this->startNetwork();
            return;
        } else {
            mainApp->CallForRender(); // If we re-render a few times during this process the main screen won't flicker
            sourceString = "inst.net.source_string"_lang;
            this->pageInfoText->SetText("inst.net.top_info"_lang);
            this->butText->SetText("inst.net.buttons1"_lang);
            this->drawMenuItems(true);
            this->menu->SetSelectedIndex(0);
            mainApp->CallForRender();
            this->infoImage->SetVisible(false);
            this->menu->SetVisible(true);
        }
        return;
    }

    void netInstPage::startInstall(bool urlMode) {
        int dialogResult = -1;
        if (this->selectedUrls.size() == 1) {
            std::string ourUrlString;
            if (this->alternativeNames.size() > 0) ourUrlString = inst::util::shortenString(this->alternativeNames[0], 32, true);
            else ourUrlString = inst::util::shortenString(inst::util::formatUrlString(this->selectedUrls[0]), 32, true);
            dialogResult = mainApp->CreateShowDialog("inst.target.desc0"_lang + ourUrlString + "inst.target.desc1"_lang, "common.cancel_desc"_lang, {"inst.target.opt0"_lang, "inst.target.opt1"_lang}, false);
        } else dialogResult = mainApp->CreateShowDialog("inst.target.desc00"_lang + std::to_string(this->selectedUrls.size()) + "inst.target.desc01"_lang, "common.cancel_desc"_lang, {"inst.target.opt0"_lang, "inst.target.opt1"_lang}, false);
        if (dialogResult == -1 && !urlMode) return;
        else if (dialogResult == -1 && urlMode) {
            this->startNetwork();
            return;
        }
        netInstStuff::installTitleNet(this->selectedUrls, dialogResult, this->alternativeNames, sourceString);
        return;
    }

    void netInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & HidNpadButton_B) {
            if (this->menu->GetItems().size() > 0){
                if (this->selectedUrls.size() == 0) {
                    this->selectTitle(this->menu->GetSelectedIndex());
                }
                netInstStuff::sendExitCommands(inst::util::formatUrlLink(this->selectedUrls[0]));
            }
            netInstStuff::OnUnwound();
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if ((Down & HidNpadButton_A) || (Up & TouchPseudoKey)) {
            this->selectTitle(this->menu->GetSelectedIndex());
            if (this->menu->GetItems().size() == 1 && this->selectedUrls.size() == 1) {
                this->startInstall(false);
            }
        }
        if ((Down & HidNpadButton_Y)) {
            if (this->selectedUrls.size() == this->menu->GetItems().size()) this->drawMenuItems(true);
            else {
                for (long unsigned int i = 0; i < this->menu->GetItems().size(); i++) {
                    if (this->menu->GetItems()[i]->GetIcon() == "romfs:/images/icons/check-box-outline.png") continue;
                    else this->selectTitle(i);
                }
                this->drawMenuItems(false);
            }
        }
        if (Down & HidNpadButton_Plus) {
            if (this->selectedUrls.size() == 0) {
                this->selectTitle(this->menu->GetSelectedIndex());
            }
            this->startInstall(false);
        }
    }
}
