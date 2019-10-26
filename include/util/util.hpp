#pragma once
#include <string>
#include <filesystem>

namespace util {
    void initApp ();
    void deinitApp ();
    std::vector<std::filesystem::path> getDirectoryFiles(const std::string & dir, const std::vector<std::string> & extensions);
    bool removeDirectory(std::string dir);
    bool copyFile(std::string inFile, std::string outFile);
}