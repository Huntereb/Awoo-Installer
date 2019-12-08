#pragma once
#include <vector>
#include <string>

namespace usbInstStuff {
    int OnSelected(void* in);
    void installTitleUsb(std::vector<std::string> ourNspList, int ourStorage);
    extern bool stopThread;
}