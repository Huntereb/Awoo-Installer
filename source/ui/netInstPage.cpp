#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/mainPage.hpp"
#include "ui/netInstPage.hpp"
#include "util.hpp"
#include "netInstall.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;
    extern MainApplication *netinstPage;

    std::vector<std::string> ourUrls;

    netInstPage::netInstPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->topText = TextBlock::New(10, 2, "Awoo Installer", 35);
        this->topText->SetColor(COLOR("#FFFFFFFF"));
        this->pageInfoText = TextBlock::New(10, 45, "", 35);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->Add(this->topText);
        this->Add(this->pageInfoText);
    }

    void netInstPage::startInstall() {
        mainApp->LoadLayout(mainApp->netinstPage);
        mainApp->CallForRender();
        //gets our list of urls of nsps
        ourUrls = netInstStuff::OnSelected();
        if (ourUrls.size() == 0) {
            return;
        }
        //make it so we fill a page with nsps here?
        std::string ourSelectedNsp = "installing: " + ourUrls[0];
        this->pageInfoText->SetText(ourSelectedNsp);
        mainApp->CallForRender();
        //automatically selecting first nsp and SD as storage
        if (netInstStuff::OnNSPSelected(ourUrls[0], 1)) {
            mainApp->CreateShowDialog(ourUrls[0] + " installed!", "", {"OK"}, true);
            mainApp->LoadLayout(mainApp->mainPage);
        }
        return;
    }

    void netInstPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
        if (Down & KEY_B) {
            mainApp->LoadLayout(mainApp->mainPage);
        }
    }
}
