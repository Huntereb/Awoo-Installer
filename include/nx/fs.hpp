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

#pragma once

#include <string>

extern "C"
{
#include <switch/types.h>
#include <switch/services/fs.h>
}

#include "nx/ipc/tin_ipc.h"

namespace nx::fs
{
    class IFileSystem;

    class IFile
    {
        friend IFileSystem;

        private:
            FsFile m_file;

            IFile(FsFile& file);

        public:
            // Don't allow copying, or garbage may be closed by the destructor
            IFile& operator=(const IFile&) = delete;
            IFile(const IFile&) = delete;   

            ~IFile();

            void Read(u64 offset, void* buf, size_t size);
            s64 GetSize();
    };

    class IDirectory
    {
        friend IFileSystem;

        private:
            FsDir m_dir;

            IDirectory(FsDir& dir);

        public:
            // Don't allow copying, or garbage may be closed by the destructor
            IDirectory& operator=(const IDirectory&) = delete;
            IDirectory(const IDirectory&) = delete;   

            ~IDirectory();

            void Read(s64 inval, FsDirectoryEntry* buf, size_t numEntries);
            u64 GetEntryCount();
    };

    class IFileSystem
    {
        private:
            FsFileSystem m_fileSystem;

        public:
            // Don't allow copying, or garbage may be closed by the destructor
            IFileSystem& operator=(const IFileSystem&) = delete;
            IFileSystem(const IFileSystem&) = delete;   

            IFileSystem();
            ~IFileSystem();

            Result OpenSdFileSystem();
            void OpenFileSystemWithId(std::string path, FsFileSystemType fileSystemType, u64 titleId);
            void CloseFileSystem();
             
            IFile OpenFile(std::string path);
            IDirectory OpenDirectory(std::string path, int flags);
    };
}