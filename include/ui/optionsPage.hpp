#pragma once
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace inst::ui {
    class optionsPage : public pu::ui::Layout
    {
        public:
            optionsPage();
            PU_SMART_CTOR(optionsPage)
            void onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos);
        private:
            TextBlock::Ref butText;
            Rectangle::Ref topRect;
            Rectangle::Ref infoRect;
            Rectangle::Ref botRect;
            Image::Ref titleImage;
            TextBlock::Ref pageInfoText;
            pu::ui::elm::Menu::Ref menu;
            void setMenuText();
            std::string getMenuOptionIcon(bool ourBool);
    };
}