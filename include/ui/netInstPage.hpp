#pragma once
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace inst::ui {
    class netInstPage : public pu::ui::Layout
    {
        public:
            netInstPage();
            PU_SMART_CTOR(netInstPage)
            void startInstall(bool urlMode);
            void startNetwork();
            void onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos);
            TextBlock::Ref pageInfoText;
        private:
            static std::vector<std::string> ourUrls;
            static std::vector<std::string> selectedUrls;
            static std::vector<std::string> alternativeNames;
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