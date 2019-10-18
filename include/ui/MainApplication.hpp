#pragma once
#include <pu/Plutonium>
#include "ui/mainPage.hpp"
#include "ui/netInstPage.hpp"

namespace inst::ui {
    class MainApplication : public pu::ui::Application {
        public:
            using Application::Application;
            PU_SMART_CTOR(MainApplication)
            void OnLoad() override;
            MainPage::Ref mainPage;
            netInstPage::Ref netinstPage;
    };
}