#pragma once
#include <filesystem>

namespace inst::util {
    void initApp ();
    void deinitApp ();
    void initInstallServices();
    void deinitInstallServices();
    std::vector<std::filesystem::path> getDirectoryFiles(const std::string & dir, const std::vector<std::string> & extensions);
    bool removeDirectory(std::string dir);
    bool copyFile(std::string inFile, std::string outFile);
    std::string formatUrlString(std::string ourString);
    std::string shortenString(std::string ourString, int ourLength, bool isFile);
    std::string readTextFromFile(std::string ourFile);
    std::string softwareKeyboard(std::string guideText, std::string initialText, int LenMax);
    std::string getDriveFileName(std::string fileId);
}