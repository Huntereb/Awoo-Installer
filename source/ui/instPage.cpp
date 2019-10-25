#include <filesystem>
#include "ui/MainApplication.hpp"
#include "ui/instPage.hpp"

#define COLOR(hex) pu::ui::Color::FromHex(hex)

namespace inst::ui {
    extern MainApplication *mainApp;

    instPage::instPage() : Layout::Layout() {
        this->SetBackgroundColor(COLOR("#670000FF"));
        this->topText = TextBlock::New(10, 2, "Awoo Installer", 35);
        this->topText->SetColor(COLOR("#FFFFFFFF"));
        this->pageInfoText = TextBlock::New(10, 180, "", 35);
        this->pageInfoText->SetColor(COLOR("#FFFFFFFF"));
        this->installBar = pu::ui::elm::ProgressBar::New(340, 360, 600, 48, 100.0f);
        this->installBar->SetColor(COLOR("#222222FF"));
        this->Add(this->topText);
        this->Add(this->pageInfoText);
        this->Add(this->installBar);
    }

    void instPage::onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos) {
    }
}
