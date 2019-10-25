#pragma once
#include <string>

namespace inst::ui {
    void setInstInfoText(std::string ourText);
    void setInstBarPerc(double ourPercent);
    void hideInstBar(bool hidden);
    void loadMainMenu();
    void loadInstallScreen();
}

namespace nspInstStuff {
    void installNspFromFile(std::string ourNsp, int whereToInstall);
}