#pragma once
#include <string>

namespace inst::ui {
    void setInstInfoText(std::string ourText);
    void loadMainMenu();
    void loadInstallScreen();
}

namespace nspInstStuff {
    void installNspFromFile(std::string ourNsp, int whereToInstall);
}