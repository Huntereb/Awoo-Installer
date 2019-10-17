#include "ui/MainApplication.hpp"

namespace inst::ui {
    MainApplication *mainApp;

    void MainApplication::OnLoad() {
        mainApp = this;

        this->mainPage = MainPage::New();
        this->mainPage->SetOnInput(std::bind(&MainPage::onInput, this->mainPage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        this->LoadLayout(this->mainPage);
    }
}