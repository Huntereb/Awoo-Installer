#include <filesystem>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/netInstPage.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "util/curl.hpp"
#include "netInstall.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    std::vector<std::string> netInstPage::ourUrls;
    std::vector<std::string> netInstPage::selectedUrls;
    std::vector<std::string> netInstPage::alternativeNames;
    std::string lastUrl = "https://";
    std::string lastFileID = "";

    netInstPage::netInstPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->SetBackgroundImage("romfs:/background.jpg");
        this->topRect = Rectangle::New(0, 0, 1280, 93, COLOR("#170909FF"));
        this->infoRect = Rectangle::New(0, 93, 1280, 60, COLOR("#17090980"));
        this->botRect = Rectangle::New(0, 660, 1280, 60, COLOR("#17090980"));
        this->titleImage = Image::New(0, 0, "romfs:/logo.png");
        this->appVersionText = TextBlock::New(480, 49, "v" + inst::config::appVersion, 22);
        this->appVersionText->SetColor(COLOR("#FFFFFFFF"));
        this->pageInfoText = TextBlock::New(10, 109, "", 30);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->butText = TextBlock::New(10, 678, "", 24);
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
        this->Add(this->menu);
    }

    void netInstPage::drawMenuItems(bool clearItems) {
        if (clearItems) netInstPage::selectedUrls = {};
        if (clearItems) netInstPage::alternativeNames = {};
        this->menu->ClearItems();
        for (auto& url: netInstPage::ourUrls) {
            pu::String itm = inst::util::shortenString(inst::util::formatUrlString(url), 56, true);
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(COLOR("#FFFFFFFF"));
            ourEntry->SetIcon("romfs:/checkbox-blank-outline.png");
            for (long unsigned int i = 0; i < netInstPage::selectedUrls.size(); i++) {
                if (netInstPage::selectedUrls[i] == url) {
                    ourEntry->SetIcon("romfs:/check-box-outline.png");
                }
            }
            this->menu->AddItem(ourEntry);
        }
    }

    void netInstPage::selectNsp(int selectedIndex) {
        if (this->menu->GetItems()[selectedIndex]->GetIcon() == "romfs:/check-box-outline.png") {
            for (long unsigned int i = 0; i < netInstPage::selectedUrls.size(); i++) {
                if (netInstPage::selectedUrls[i] == netInstPage::ourUrls[selectedIndex]) netInstPage::selectedUrls.erase(netInstPage::selectedUrls.begin() + i);
            }
        } else netInstPage::selectedUrls.push_back(netInstPage::ourUrls[selectedIndex]);
        netInstPage::drawMenuItems(false);
    }

    void netInstPage::startNetwork() {
        this->pageInfoText->SetText("");
        this->butText->SetText("\ue0e3 Install Over Internet    \ue0e2 Help    \ue0e1 Cancel ");
        this->menu->SetVisible(false);
        this->menu->ClearItems();
        mainApp->LoadLayout(mainApp->netinstPage);
        netInstPage::ourUrls = netInstStuff::OnSelected();
        if (!netInstPage::ourUrls.size()) {
            mainApp->LoadLayout(mainApp->mainPage);
            return;
        } else if (netInstPage::ourUrls[0] == "supplyUrl") {
            std::string keyboardResult;
            switch (mainApp->CreateShowDialog("Where do you want to install from?", "Press B to cancel", {"URL", "Google Drive"}, false)) {
                case 0:
                    keyboardResult = inst::util::softwareKeyboard("Enter the Internet address of a file", lastUrl, 500);
                    if (keyboardResult.size() > 0) {
                        lastUrl = keyboardResult;
                        if (inst::util::formatUrlString(keyboardResult) == "" || keyboardResult == "https://" || keyboardResult == "http://") {
                            mainApp->CreateShowDialog("The URL specified is invalid!", "", {"OK"}, false);
                            break;
                        }
                        netInstPage::selectedUrls = {keyboardResult};
                        netInstPage::startInstall(true);
                        return;
                    }
                    break;
                case 1:
                    keyboardResult = inst::util::softwareKeyboard("Enter the file ID of a public Google Drive file", lastFileID, 50);
                    if (keyboardResult.size() > 0) {
                        lastFileID = keyboardResult;
                        std::string fileName = inst::util::getDriveFileName(keyboardResult);
                        if (fileName.size() > 0) netInstPage::alternativeNames = {fileName};
                        else netInstPage::alternativeNames = {"Google Drive File"};
                        netInstPage::selectedUrls = {"https://www.googleapis.com/drive/v3/files/" + keyboardResult + "?key=" + inst::config::gAuthKey + "&alt=media"};
                        netInstPage::startInstall(true);
                        return;
                    }
                    break;
            }
            netInstPage::startNetwork();
            return;
        } else {
            this->pageInfoText->SetText("Select what files you want to install from the server, then press the Plus button!");
            this->butText->SetText("\ue0e0 Select File    \ue0e3 Select All    \ue0ef Install File(s)    \ue0e1 Cancel ");
            netInstPage::drawMenuItems(true);
        }
        this->menu->SetVisible(true);
        return;
    }

    void netInstPage::startInstall(bool urlMode) {
        int dialogResult = -1;
        if (netInstPage::selectedUrls.size() == 1) {
            std::string ourUrlString;
            if (netInstPage::alternativeNames.size() > 0) ourUrlString = inst::util::shortenString(netInstPage::alternativeNames[0], 32, true);
            else ourUrlString = inst::util::shortenString(inst::util::formatUrlString(netInstPage::selectedUrls[0]), 32, true);
            dialogResult = mainApp->CreateShowDialog("Where should " + ourUrlString + " be installed to?", "Press B to cancel", {"SD Card", "Internal Storage"}, false);
        } else dialogResult = mainApp->CreateShowDialog("Where should the selected " + std::to_string(netInstPage::selectedUrls.size()) + " files be installed to?", "Press B to cancel", {"SD Card", "Internal Storage"}, false);
        if (dialogResult == -1 && !urlMode) return;
        else if (dialogResult == -1 && urlMode) {
            netInstPage::startNetwork();
            return;
        }
        netInstStuff::installNspLan(netInstPage::selectedUrls, dialogResult, netInstPage::alternativeNames);
        return;
    }

    void netInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & KEY_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if ((Down & KEY_A) || (Up & KEY_TOUCH)) {
            netInstPage::selectNsp(this->menu->GetSelectedIndex());
            if (this->menu->GetItems().size() == 1 && netInstPage::selectedUrls.size() == 1) {
                netInstPage::startInstall(false);
            }
        }
        if ((Down & KEY_Y)) {
            if (netInstPage::selectedUrls.size() == this->menu->GetItems().size()) netInstPage::drawMenuItems(true);
            else {
                for (long unsigned int i = 0; i < this->menu->GetItems().size(); i++) {
                    if (this->menu->GetItems()[i]->GetIcon() == "romfs:/check-box-outline.png") continue;
                    else netInstPage::selectNsp(i);
                }
                netInstPage::drawMenuItems(false);
            }
        }
        if (Down & KEY_PLUS) {
            if (netInstPage::selectedUrls.size() == 0) {
                netInstPage::selectNsp(this->menu->GetSelectedIndex());
                netInstPage::startInstall(false);
                return;
            }
            netInstPage::startInstall(false);
        }
    }
}
