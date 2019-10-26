#include <minizip/unzip.h>
#include <string>

namespace inst::zip {
    bool extractFile(const std::string filename, const std::string destination);
}