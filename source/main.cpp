#include "switch.h"
#include "util/error.hpp"
#include "ui/MainApplication.hpp"
#include "util/util.hpp"

using namespace pu::ui::render;
int main(int argc, char* argv[])
{
    inst::util::initApp();
    try {
        auto renderer = Renderer::New(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC | SDL_INIT_GAMECONTROLLER,
            RendererInitOptions::RendererNoSound, RendererHardwareFlags);
        auto main = inst::ui::MainApplication::New(renderer);
        main->Prepare();
        main->ShowWithFadeIn();
    } catch (std::exception& e) {
        LOG_DEBUG("An error occurred:\n%s", e.what());
    }
    inst::util::deinitApp();
    return 0;
}
