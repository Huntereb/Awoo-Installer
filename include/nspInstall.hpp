#include <string>

namespace inst::ui {
    void setNspInfoText(std::string ourText);
    void setNetInfoText(std::string ourText);
    void loadMainMenu();
}

namespace nspInstStuff {
    bool installNspFromFile(std::string ourNsp, int whereToInstall);
}