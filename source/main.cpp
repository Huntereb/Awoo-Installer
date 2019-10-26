#include "switch.h"
#include "ui/MainApplication.hpp"
#include "util.hpp"
#include <filesystem>

using namespace pu::ui::render;
int main(int argc, char* argv[])
{
    inst::util::initApp();
    try {
        auto renderer = Renderer::New(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER,
            RendererInitOptions::RendererNoSound, RendererHardwareFlags);
        auto main = inst::ui::MainApplication::New(renderer);
        main->Prepare();
        main->Show();
    } catch (std::exception& e) {
        printf("An error occurred:\n%s\n\nPress any button to exit.", e.what());

        u64 kDown = 0;

        while (!kDown)
        {
            hidScanInput();
            kDown = hidKeysDown(CONTROLLER_P1_AUTO);
        }
    }
    inst::util::deinitApp();
    return 0;
}
