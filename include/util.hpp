#include <string>
#include <filesystem>

namespace appVariables {
    static const std::string appDir = "sdmc:/switch/AwooInstaller";
}

namespace util {
    void initApp ();
    void deinitApp ();
    std::vector<std::filesystem::path> getDirectoryFiles(const std::string & dir, const std::vector<std::string> & extensions);
}