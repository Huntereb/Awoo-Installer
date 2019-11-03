#include <filesystem>
#include <switch.h>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/netInstPage.hpp"
#include "util/util.hpp"
#include "netInstall.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    std::vector<std::string> netInstPage::ourUrls;
    std::vector<std::string> netInstPage::selectedUrls;

    netInstPage::netInstPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->SetBackgroundImage("romfs:/background.jpg");
        this->topRect = Rectangle::New(0, 0, 1280, 93, COLOR("#170909FF"));
        this->infoRect = Rectangle::New(0, 93, 1280, 60, COLOR("#17090980"));
        this->botRect = Rectangle::New(0, 660, 1280, 60, COLOR("#17090980"));
        this->titleImage = Image::New(0, 0, "romfs:/logo.png");
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
        this->Add(this->butText);
        this->Add(this->pageInfoText);
        this->Add(this->menu);
    }

    void netInstPage::drawMenuItems(bool clearItems) {
        if (clearItems) netInstPage::selectedUrls = {};
        this->menu->ClearItems();
        for (auto& url: netInstPage::ourUrls) {
            pu::String itm = inst::util::shortenString(inst::util::formatUrlString(url), 64, true);
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

    void netInstPage::selectNsp() {
        if (this->menu->GetItems()[this->menu->GetSelectedIndex()]->GetIcon() == "romfs:/check-box-outline.png") {
            for (long unsigned int i = 0; i < netInstPage::selectedUrls.size(); i++) {
                if (netInstPage::selectedUrls[i] == netInstPage::ourUrls[this->menu->GetSelectedIndex()]) netInstPage::selectedUrls.erase(netInstPage::selectedUrls.begin() + i);
            }
        } else netInstPage::selectedUrls.push_back(netInstPage::ourUrls[this->menu->GetSelectedIndex()]);
        netInstPage::drawMenuItems(false);
    }

    void netInstPage::startNetwork() {
        this->pageInfoText->SetText("");
        this->butText->SetText("\ue0e3 Install From URL    \ue0e2 Help    \ue0e1 Cancel ");
        this->menu->SetVisible(false);
        this->menu->ClearItems();
        mainApp->LoadLayout(mainApp->netinstPage);
        netInstPage::ourUrls = netInstStuff::OnSelected();
        if (!netInstPage::ourUrls.size()) {
            mainApp->LoadLayout(mainApp->mainPage);
            return;
        } else if (netInstPage::ourUrls[0] == "supplyUrl") {
            Result rc=0;
            SwkbdConfig kbd;
            char tmpoutstr[128] = {0};
            rc = swkbdCreate(&kbd, 0);
            if (R_SUCCEEDED(rc)) {
                swkbdConfigMakePresetDefault(&kbd);
                swkbdConfigSetGuideText(&kbd, "Enter the Internet address of a NSP file");
                swkbdConfigSetInitialText(&kbd, "https://");
                rc = swkbdShow(&kbd, tmpoutstr, sizeof(tmpoutstr));
                swkbdClose(&kbd);
                if (R_SUCCEEDED(rc) && tmpoutstr[0] != 0) {
                    if (inst::util::formatUrlString(tmpoutstr) == "") {
                        mainApp->CreateShowDialog("The URL specified is invalid!", "", {"OK"}, false);
                        mainApp->LoadLayout(mainApp->mainPage);
                        return;
                    }
                    netInstPage::selectedUrls = {tmpoutstr};
                    netInstPage::startInstall(true);
                    return;
                } else {
                    mainApp->LoadLayout(mainApp->mainPage);
                    return;
                }
            }
        } else {
            this->pageInfoText->SetText("Select NSP files to install from the server, then press the Plus button!");
            this->butText->SetText("\ue0e0 Select NSP    \ue0ef Install NSP(s)    \ue0e1 Cancel ");
            netInstPage::drawMenuItems(true);
        }
        this->menu->SetVisible(true);
        return;
    }

    void netInstPage::startInstall(bool urlMode) {
        int dialogResult = -1;
        if (netInstPage::selectedUrls.size() == 1) {
            dialogResult = mainApp->CreateShowDialog("Where should " + inst::util::shortenString(inst::util::formatUrlString(netInstPage::selectedUrls[0]), 64, true) + " be installed to?", "Press B to cancel", {"SD Card", "Internal Storage"}, false);
        } else dialogResult = mainApp->CreateShowDialog("Where should the selected files be installed to?", "Press B to cancel", {"SD Card", "Internal Storage"}, false);
        if (dialogResult == -1 && !urlMode) return;
        else if (dialogResult == -1 && urlMode) {
            mainApp->LoadLayout(mainApp->mainPage);
            return;
        }
        netInstStuff::installNspLan(netInstPage::selectedUrls, dialogResult);
        return;
    }

    void netInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & KEY_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if ((Down & KEY_A) || (Up & KEY_TOUCH)) {
            netInstPage::selectNsp();
        }
        if (Down & KEY_PLUS) {
            if (netInstPage::selectedUrls.size() == 0) netInstPage::selectNsp();
            netInstPage::startInstall(false);
        }
    }
}
