#pragma once
#include <pu/Plutonium>

using namespace pu::ui::elm;
namespace inst::ui {
    class instPage : public pu::ui::Layout
    {
        public:
            instPage();
            PU_SMART_CTOR(instPage)
            void onInput(u64 Down, u64 Up, u64 Held, pu::ui::Touch Pos);
            TextBlock::Ref pageInfoText;
            TextBlock::Ref installInfoText;
            pu::ui::elm::ProgressBar::Ref installBar;
            Image::Ref awooImage;
        private:
            Rectangle::Ref infoRect;
            Rectangle::Ref topRect;
            Image::Ref titleImage;
            TextBlock::Ref appVersionText;
    };
}