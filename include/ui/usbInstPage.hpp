#pragma once
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace inst::ui {
    class usbInstPage : public pu::ui::Layout
    {
        public:
            usbInstPage();
            PU_SMART_CTOR(usbInstPage)
            void startInstall();
            void startUsb();
            void onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos);
            TextBlock::Ref pageInfoText;
            Image::Ref titleImage;
            TextBlock::Ref appVersionText;
        private:
            std::vector<std::string> ourTitles;
            std::vector<std::string> selectedTitles;
            std::string lastUrl;
            std::string lastFileID;
            TextBlock::Ref butText;
            Rectangle::Ref topRect;
            Rectangle::Ref infoRect;
            Rectangle::Ref botRect;
            pu::ui::elm::Menu::Ref menu;
            Image::Ref infoImage;
            void drawMenuItems(bool clearItems);
            void selectTitle(int selectedIndex);
    };
} 