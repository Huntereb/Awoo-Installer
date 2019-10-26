#pragma once
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace inst::ui {
    class MainPage : public pu::ui::Layout
    {
        public:
            MainPage();
            PU_SMART_CTOR(MainPage)
            void installMenuItem_Click();
            void netInstallMenuItem_Click();
            void sigPatchesMenuItem_Click();
            void settingsMenuItem_Click();
            void exitMenuItem_Click();
            void onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos);
        private:
            TextBlock::Ref topText;
            pu::ui::elm::Menu::Ref optionMenu;
            pu::ui::elm::MenuItem::Ref installMenuItem;
            pu::ui::elm::MenuItem::Ref netInstallMenuItem;
            pu::ui::elm::MenuItem::Ref sigPatchesMenuItem;
            pu::ui::elm::MenuItem::Ref settingsMenuItem;
            pu::ui::elm::MenuItem::Ref exitMenuItem;
            TextBlock::Ref infoText;
            Rectangle::Ref topRect;
            TextBlock::Ref bottomText;
            Image::Ref preview;
            Image::Ref image;
            std::string url;
    };
}