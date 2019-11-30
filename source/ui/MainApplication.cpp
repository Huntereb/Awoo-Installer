#include "ui/MainApplication.hpp"

namespace inst::ui {
    MainApplication *mainApp;

    void MainApplication::OnLoad() {
        mainApp = this;

        this->mainPage = MainPage::New();
        this->netinstPage = netInstPage::New();
        this->sdinstPage = sdInstPage::New();
        this->usbinstPage = usbInstPage::New();
        this->instpage = instPage::New();
        this->optionspage = optionsPage::New();
        this->mainPage->SetOnInput(std::bind(&MainPage::onInput, this->mainPage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        this->netinstPage->SetOnInput(std::bind(&netInstPage::onInput, this->netinstPage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        this->sdinstPage->SetOnInput(std::bind(&sdInstPage::onInput, this->sdinstPage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        this->usbinstPage->SetOnInput(std::bind(&usbInstPage::onInput, this->usbinstPage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        this->instpage->SetOnInput(std::bind(&instPage::onInput, this->instpage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        this->optionspage->SetOnInput(std::bind(&optionsPage::onInput, this->optionspage, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        this->LoadLayout(this->mainPage);
    }
}