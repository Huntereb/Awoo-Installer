#include "ui/usbInstPage.hpp"
#include "ui/MainApplication.hpp"
#include "util/util.hpp"
#include "util/config.hpp"
#include "usbInstall.hpp"


#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    std::vector<std::string> usbInstPage::ourNsps;
    std::vector<std::string> usbInstPage::selectedNsps;

    usbInstPage::usbInstPage() : Layout::Layout() {
        usbCommsInitialize();
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

    void usbInstPage::drawMenuItems(bool clearItems) {
        if (clearItems) usbInstPage::selectedNsps = {};
        this->menu->ClearItems();
        for (auto& url: usbInstPage::ourNsps) {
            pu::String itm = inst::util::shortenString(inst::util::formatUrlString(url), 56, true);
            auto ourEntry = pu::ui::elm::MenuItem::New(itm);
            ourEntry->SetColor(COLOR("#FFFFFFFF"));
            ourEntry->SetIcon("romfs:/checkbox-blank-outline.png");
            for (long unsigned int i = 0; i < usbInstPage::selectedNsps.size(); i++) {
                if (usbInstPage::selectedNsps[i] == url) {
                    ourEntry->SetIcon("romfs:/check-box-outline.png");
                }
            }
            this->menu->AddItem(ourEntry);
        }
    }

    void usbInstPage::selectNsp(int selectedIndex) {
        if (this->menu->GetItems()[selectedIndex]->GetIcon() == "romfs:/check-box-outline.png") {
            for (long unsigned int i = 0; i < usbInstPage::selectedNsps.size(); i++) {
                if (usbInstPage::selectedNsps[i] == usbInstPage::ourNsps[selectedIndex]) usbInstPage::selectedNsps.erase(usbInstPage::selectedNsps.begin() + i);
            }
        } else usbInstPage::selectedNsps.push_back(usbInstPage::ourNsps[selectedIndex]);
        usbInstPage::drawMenuItems(false);
    }

    void usbInstPage::startUsb() {
        this->pageInfoText->SetText("");
        this->butText->SetText("\ue0e2 Help    \ue0e1 Cancel ");
        this->menu->SetVisible(false);
        this->menu->ClearItems();
        mainApp->LoadLayout(mainApp->usbinstPage);
        usbInstPage::ourNsps = usbInstStuff::OnSelected();
        if (!usbInstPage::ourNsps.size()) {
            mainApp->LoadLayout(mainApp->mainPage);
            return;
        } else {
            this->pageInfoText->SetText("Select what files you want to install from the usb, then press the Plus button!");
            this->butText->SetText("\ue0e0 Select File    \ue0e3 Select All    \ue0ef Install File(s)    \ue0e1 Cancel ");
            usbInstPage::drawMenuItems(true);
        }
        this->menu->SetVisible(true);
        return;
    }

    void usbInstPage::startInstall() {
        int dialogResult = -1;
        if (usbInstPage::selectedNsps.size() == 1) dialogResult = mainApp->CreateShowDialog("Where should " + selectedNsps[0] + " be installed to?", "Press B to cancel", {"SD Card", "Internal Storage"}, false);
        else dialogResult = mainApp->CreateShowDialog("Where should the selected " + std::to_string(usbInstPage::selectedNsps.size()) + " files be installed to?", "Press B to cancel", {"SD Card", "Internal Storage"}, false);
        if (dialogResult == -1) return;
        usbInstStuff::installNspUsb(usbInstPage::selectedNsps, dialogResult);
        return;
    }

    void usbInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & KEY_B) {
            usbCommsExit();
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if ((Down & KEY_A) || (Up & KEY_TOUCH)) {
            usbInstPage::selectNsp(this->menu->GetSelectedIndex());
            if (this->menu->GetItems().size() == 1 && usbInstPage::selectedNsps.size() == 1) {
                usbInstPage::startInstall();
            }
        }
        if ((Down & KEY_Y)) {
            if (usbInstPage::selectedNsps.size() == this->menu->GetItems().size()) usbInstPage::drawMenuItems(true);
            else {
                for (long unsigned int i = 0; i < this->menu->GetItems().size(); i++) {
                    if (this->menu->GetItems()[i]->GetIcon() == "romfs:/check-box-outline.png") continue;
                    else usbInstPage::selectNsp(i);
                }
                usbInstPage::drawMenuItems(false);
            }
        }
        if (Down & KEY_PLUS) {
            if (usbInstPage::selectedNsps.size() == 0) {
                usbInstPage::selectNsp(this->menu->GetSelectedIndex());
                usbInstPage::startInstall();
                return;
            }
            usbInstPage::startInstall();
        }
    }
}