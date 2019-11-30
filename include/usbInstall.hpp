#pragma once
#include <vector>
#include <string>

namespace usbInstStuff {
    std::vector<std::string> OnSelected();
    void installTitleUsb(std::vector<std::string> ourNspList, int ourStorage);
}