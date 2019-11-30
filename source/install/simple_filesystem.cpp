/*
Copyright (c) 2017-2018 Adubbz

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "install/simple_filesystem.hpp"

#include <exception>
#include <memory>
#include "nx/fs.hpp"
#include "util/error.hpp"

namespace tin::install::nsp
{
    SimpleFileSystem::SimpleFileSystem(nx::fs::IFileSystem& fileSystem, std::string rootPath, std::string absoluteRootPath) :
        m_fileSystem(&fileSystem) , m_rootPath(rootPath), m_absoluteRootPath(absoluteRootPath)
    {}

    SimpleFileSystem::~SimpleFileSystem() {}

    nx::fs::IFile SimpleFileSystem::OpenFile(std::string path)
    {
        return m_fileSystem->OpenFile(m_rootPath + path);
    }

    bool SimpleFileSystem::HasFile(std::string path)
    {
        try
        {
            LOG_DEBUG(("Attempting to find file at " + m_rootPath + path + "\n").c_str());
            m_fileSystem->OpenFile(m_rootPath + path);
            return true;
        }
        catch (std::exception& e) {}
        return false;
    }

    std::string SimpleFileSystem::GetFileNameFromExtension(std::string path, std::string extension)
    {
        nx::fs::IDirectory dir = m_fileSystem->OpenDirectory(m_rootPath + path, FsDirOpenMode_ReadFiles | FsDirOpenMode_ReadDirs);

        u64 entryCount = dir.GetEntryCount();
        auto dirEntries = std::make_unique<FsDirectoryEntry[]>(entryCount);

        dir.Read(0, dirEntries.get(), entryCount);

        for (unsigned int i = 0; i < entryCount; i++)
        {
            FsDirectoryEntry dirEntry = dirEntries[i];
            std::string dirEntryName = dirEntry.name;

            if (dirEntry.type == FsDirEntryType_Dir)
            {
                auto subdirPath = path + dirEntryName + "/";
                auto subdirFound = this->GetFileNameFromExtension(subdirPath, extension);

                if (subdirFound != "")
                    return subdirFound;
                continue;
            }
            else if (dirEntry.type == FsDirEntryType_File)
            {
                auto foundExtension = dirEntryName.substr(dirEntryName.find(".") + 1); 

                if (foundExtension == extension)
                    return dirEntryName;
            }
        }

        return "";
    }
}