#pragma once
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
        private:
            TextBlock::Ref topText;
            pu::ui::elm::Menu::Ref menu;
    };
}