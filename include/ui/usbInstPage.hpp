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
        private:
            static std::vector<std::string> ourNsps;
            static std::vector<std::string> selectedNsps;
            std::string lastUrl;
            std::string lastFileID;
            TextBlock::Ref butText;
            Rectangle::Ref topRect;
            Rectangle::Ref infoRect;
            Rectangle::Ref botRect;
            Image::Ref titleImage;
            TextBlock::Ref appVersionText;
            pu::ui::elm::Menu::Ref menu;
            void drawMenuItems(bool clearItems);
            void selectNsp(int selectedIndex);
    };
} 