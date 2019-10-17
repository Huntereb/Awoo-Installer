#include "switch.h"
#include "ui/MainApplication.hpp"
#include "main.hpp"
#include <filesystem>

using namespace pu::ui::render;
int main(int argc, char* argv[])
{
    socketInitializeDefault();
    #ifdef __DEBUG__
        nxlinkStdio();
    #endif
    printf("Application started!\n");
    try {
        if (!std::filesystem::exists("sdmc:/switch")) std::filesystem::create_directory("sdmc:/switch");
        if (!std::filesystem::exists(appVariables::appDir)) std::filesystem::create_directory(appVariables::appDir);
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

    printf("Application closing!\n");
    socketExit();
    return 0;
}
