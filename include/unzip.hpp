extern "C" {
    #include <minizip/unzip.h>
}

namespace zipStuff {
    bool extractFile(const std::string filename, const std::string destination);
}