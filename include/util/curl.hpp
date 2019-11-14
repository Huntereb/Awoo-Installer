#include <string>

namespace inst::curl {
    bool downloadFile(const std::string ourUrl, const char *pagefilename);
    std::string downloadToBuffer (const std::string ourUrl);
}