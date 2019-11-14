#pragma once
#include <filesystem>
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace inst::ui {
    class nspInstPage : public pu::ui::Layout
    {
        public:
            nspInstPage();
            PU_SMART_CTOR(nspInstPage)
            void startInstall();
            void onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos);
            TextBlock::Ref pageInfoText;
            void drawMenuItems(bool clearItems);
        private:
            static std::vector<std::filesystem::path> ourFiles;
            static std::vector<std::filesystem::path> selectedNsps;
            TextBlock::Ref butText;
            Rectangle::Ref topRect;
            Rectangle::Ref infoRect;
            Rectangle::Ref botRect;
            Image::Ref titleImage;
            TextBlock::Ref appVersionText;
            pu::ui::elm::Menu::Ref menu;
            void selectNsp(int selectedIndex);
    };
}