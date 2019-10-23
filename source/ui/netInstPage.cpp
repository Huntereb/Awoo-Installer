#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/netInstPage.hpp"
#include "util.hpp"
#include "netInstall.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    std::vector<std::string> ourUrls;

    netInstPage::netInstPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->topText = TextBlock::New(10, 2, "Awoo Installer", 35);
        this->topText->SetColor(COLOR("#FFFFFFFF"));
        this->pageInfoText = TextBlock::New(10, 45, "", 35);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->menu = pu::ui::elm::Menu::New(0, 160, 1280, COLOR("#FFFFFF00"), 80, (560 / 80));
        this->menu->SetOnFocusColor(COLOR("#00000033"));
        this->Add(this->topText);
        this->Add(this->pageInfoText);
        this->Add(this->menu);
    }

    void netInstPage::startNetwork() {
        this->menu->SetVisible(false);
        this->menu->ClearItems();
        mainApp->LoadLayout(mainApp->netinstPage);
        mainApp->CallForRender();
        ourUrls = netInstStuff::OnSelected();
        if (!ourUrls.size()) {
            return;
        } else {
            for (auto& url: ourUrls) {
                pu::String itm = url;
                auto ourEntry = pu::ui::elm::MenuItem::New(itm);
                ourEntry->SetColor(COLOR("#FFFFFFFF"));
                this->menu->AddItem(ourEntry);
            }
        }
        this->menu->SetVisible(true);
        this->pageInfoText->SetText("Select a NSP to install! Press B to cancel!");
        return;
    }

    void netInstPage::startInstall() {
        std::string ourUrl = ourUrls[this->menu->GetSelectedIndex()];
        int dialogResult = mainApp->CreateShowDialog("Where should " + ourUrl + " be installed to?", "Press B to cancel", {"SD", "Internal Storage"}, false);
        if (dialogResult == -1) return;
        this->pageInfoText->SetText("installing: " + ourUrl);
        mainApp->CallForRender();
        if (netInstStuff::installNspLan(ourUrl, dialogResult)) {
            mainApp->CreateShowDialog(ourUrls[0] + " installed!", "", {"OK"}, true);
        }
        return;
    }

    void netInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & KEY_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
        if (Down & KEY_A) {
            if (this->menu->IsVisible()) startInstall();
        }
    }
}
