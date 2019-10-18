#pragma once
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace inst::ui {
    class netInstPage : public pu::ui::Layout
    {
        public:
            netInstPage();
            PU_SMART_CTOR(netInstPage)
            void onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos);
            TextBlock::Ref pageInfoText;
        private:
            TextBlock::Ref topText;
    };
}