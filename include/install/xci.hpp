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

#include <functional>
#include <vector>

#include <switch/types.h>
#include "install/hfs0.hpp"
#include "nx/ncm.hpp"
#include <memory>

namespace tin::install::xci
{
    class XCI
    {
        protected:
            u64 m_secureHeaderOffset;
            std::vector<u8> m_secureHeaderBytes;

            XCI();

        public:
            virtual void StreamToPlaceholder(std::shared_ptr<nx::ncm::ContentStorage>& contentStorage, NcmContentId placeholderId) = 0;
            virtual void BufferData(void* buf, off_t offset, size_t size) = 0;

            virtual void RetrieveHeader();
            virtual const HFS0BaseHeader* GetSecureHeader();
            virtual u64 GetDataOffset();

            virtual const HFS0FileEntry* GetFileEntry(unsigned int index);
            virtual const HFS0FileEntry* GetFileEntryByName(std::string name);
            virtual const HFS0FileEntry* GetFileEntryByNcaId(const NcmContentId& ncaId);
            virtual std::vector<const HFS0FileEntry*> GetFileEntriesByExtension(std::string extension);

            virtual const char* GetFileEntryName(const HFS0FileEntry* fileEntry);
    };
}