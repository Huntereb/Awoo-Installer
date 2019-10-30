#pragma once

namespace inst::ui {
    void setTopInstInfoText(std::string ourText);
    void setInstInfoText(std::string ourText);
    void setInstBarPerc(double ourPercent);
    void hideInstBar(bool hidden);
    void loadMainMenu();
    void loadInstallScreen();
}

namespace nspInstStuff {
    void installNspFromFile(std::string ourNsp, int whereToInstall);
}