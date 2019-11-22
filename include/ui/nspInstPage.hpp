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
            pu::ui::elm::Menu::Ref menu;
            void startInstall();
            void onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos);
            TextBlock::Ref pageInfoText;
            void drawMenuItems(bool clearItems, std::filesystem::path ourPath);
        private:
            static std::vector<std::filesystem::path> ourDirectories;
            static std::vector<std::filesystem::path> ourFiles;
            static std::vector<std::filesystem::path> selectedNsps;
            static std::filesystem::path currentDir;
            TextBlock::Ref butText;
            Rectangle::Ref topRect;
            Rectangle::Ref infoRect;
            Rectangle::Ref botRect;
            Image::Ref titleImage;
            TextBlock::Ref appVersionText;
            void followDirectory();
            void selectNsp(int selectedIndex);
    };
}