#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/netInstPage.hpp"
#include "util/util.hpp"
#include "netInstall.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    std::vector<std::string> ourUrls;

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
        this->menu = pu::ui::elm::Menu::New(0, 153, 1280, COLOR("#FFFFFF00"), 84, (506 / 84));
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

    void netInstPage::startNetwork() {
        this->pageInfoText->SetText("");
        this->butText->SetText("\ue0e1 Cancel    \ue0e3 Install From URL ");
        this->menu->SetVisible(false);
        this->menu->ClearItems();
        mainApp->LoadLayout(mainApp->netinstPage);
        ourUrls = netInstStuff::OnSelected();
        if (!ourUrls.size()) {
            mainApp->LoadLayout(mainApp->mainPage);
            return;
        } else if (ourUrls[0] == "supplyUrl") {
            Result rc=0;
            SwkbdConfig kbd;
            char tmpoutstr[128] = {0};
            rc = swkbdCreate(&kbd, 0);
            if (R_SUCCEEDED(rc)) {
                swkbdConfigMakePresetDefault(&kbd);
                swkbdConfigSetGuideText(&kbd, "Enter the location of a NSP! URL must be HTTP.");
                swkbdConfigSetInitialText(&kbd, "http://");
                rc = swkbdShow(&kbd, tmpoutstr, sizeof(tmpoutstr));
                swkbdClose(&kbd);
                if (R_SUCCEEDED(rc) && tmpoutstr[0] != 0) {
                    ourUrls[0] = tmpoutstr;
                    netInstPage::startInstall(true);
                    return;
                } else {
                    mainApp->LoadLayout(mainApp->mainPage);
                    return;
                }
            }
        } else {
            this->pageInfoText->SetText("Select a NSP to install!");
            this->butText->SetText("\ue0e0 Install NSP    \ue0e1 Cancel ");
            for (auto& url: ourUrls) {
                pu::String itm = inst::util::shortenString(inst::util::formatUrlString(url), 64, true);
                auto ourEntry = pu::ui::elm::MenuItem::New(itm);
                ourEntry->SetColor(COLOR("#FFFFFFFF"));
                ourEntry->SetIcon("romfs:/package-down.png");
                this->menu->AddItem(ourEntry);
            }
        }
        this->menu->SetVisible(true);
        return;
    }

    void netInstPage::startInstall(bool urlMode) {
        std::string ourUrl = ourUrls[this->menu->GetSelectedIndex()];
        int dialogResult = mainApp->CreateShowDialog("Where should " + inst::util::shortenString(inst::util::formatUrlString(ourUrl), 64, true) + " be installed to?", "Press B to cancel", {"SD Card", "Internal Storage"}, false);
        if (dialogResult == -1 && !urlMode) return;
        else if (dialogResult == -1 && urlMode) {
            mainApp->LoadLayout(mainApp->mainPage);
            return;
        }
        netInstStuff::installNspLan(ourUrl, dialogResult);
        return;
    }

    void netInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & KEY_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if (Down & KEY_A) {
            if (this->menu->IsVisible()) startInstall(false);
        }
    }
}
