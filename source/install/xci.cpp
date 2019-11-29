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

#include "install/xci.hpp"
#include "util/title_util.hpp"
#include "error.hpp"
#include "debug.h"

namespace tin::install::xci
{
    XCI::XCI()
    {
    }

    void XCI::RetrieveHeader()
    {
        printf("Retrieving HFS0 header...\n");

        // Retrieve hfs0 offset
        u64 hfs0Offset = 0xf000;

        // Retrieve main hfs0 header
        std::vector<u8> m_headerBytes;
        m_headerBytes.resize(sizeof(HFS0BaseHeader), 0);
        this->BufferData(m_headerBytes.data(), hfs0Offset, sizeof(HFS0BaseHeader));

        printf("Base header: \n");
        printBytes(nxlinkout, m_headerBytes.data(), sizeof(HFS0BaseHeader), true);

        // Retrieve full header
        HFS0BaseHeader *header = reinterpret_cast<HFS0BaseHeader*>(m_headerBytes.data());
        if (header->magic != MAGIC_HFS0)
            THROW_FORMAT("hfs0 magic doesn't match at 0x%lx\n", hfs0Offset);

        size_t remainingHeaderSize = header->numFiles * sizeof(HFS0FileEntry) + header->stringTableSize;
        m_headerBytes.resize(sizeof(HFS0BaseHeader) + remainingHeaderSize, 0);
        this->BufferData(m_headerBytes.data() + sizeof(HFS0BaseHeader), hfs0Offset + sizeof(HFS0BaseHeader), remainingHeaderSize);

        printf("Base header: \n");
        printBytes(nxlinkout, m_headerBytes.data(), sizeof(HFS0BaseHeader) + remainingHeaderSize, true);

        // Find Secure partition
        header = reinterpret_cast<HFS0BaseHeader*>(m_headerBytes.data());
        for (unsigned int i = 0; i < header->numFiles; i++)
        {
            const HFS0FileEntry *entry = hfs0GetFileEntry(header, i);
            std::string entryName(hfs0GetFileName(header, entry));

            if (entryName != "secure")
                continue;

            m_secureHeaderOffset = hfs0Offset + remainingHeaderSize + 0x10 + entry->dataOffset;
            m_secureHeaderBytes.resize(sizeof(HFS0BaseHeader), 0);
            this->BufferData(m_secureHeaderBytes.data(), m_secureHeaderOffset, sizeof(HFS0BaseHeader));

            printf("Secure header: \n");
            printBytes(nxlinkout, m_secureHeaderBytes.data(), sizeof(HFS0BaseHeader), true);

            if (this->GetSecureHeader()->magic != MAGIC_HFS0)
                THROW_FORMAT("hfs0 magic doesn't match at 0x%lx\n", m_secureHeaderOffset);

            // Retrieve full header
            remainingHeaderSize = this->GetSecureHeader()->numFiles * sizeof(HFS0FileEntry) + this->GetSecureHeader()->stringTableSize;
            m_secureHeaderBytes.resize(sizeof(HFS0BaseHeader) + remainingHeaderSize, 0);
            this->BufferData(m_secureHeaderBytes.data() + sizeof(HFS0BaseHeader), m_secureHeaderOffset + sizeof(HFS0BaseHeader), remainingHeaderSize);
            return;
        }
        THROW_FORMAT("couldn't optain secure hfs0 header\n");
    }

    const HFS0BaseHeader* XCI::GetSecureHeader()
    {
        if (m_secureHeaderBytes.empty())
            THROW_FORMAT("Cannot retrieve header as header bytes are empty. Have you retrieved it yet?\n");

        return reinterpret_cast<HFS0BaseHeader*>(m_secureHeaderBytes.data());
    }

    u64 XCI::GetDataOffset()
    {
        if (m_secureHeaderBytes.empty())
            THROW_FORMAT("Cannot get data offset as header is empty. Have you retrieved it yet?\n");

        return m_secureHeaderOffset + m_secureHeaderBytes.size();
    }

    const HFS0FileEntry* XCI::GetFileEntry(unsigned int index)
    {
        if (index >= this->GetSecureHeader()->numFiles)
            THROW_FORMAT("File entry index is out of bounds\n")

        return hfs0GetFileEntry(this->GetSecureHeader(), index);
    }

    const HFS0FileEntry* XCI::GetFileEntryByName(std::string name)
    {
        for (unsigned int i = 0; i < this->GetSecureHeader()->numFiles; i++)
        {
            const HFS0FileEntry* fileEntry = this->GetFileEntry(i);
            std::string foundName(this->GetFileEntryName(fileEntry));

            if (foundName == name)
                return fileEntry;
        }

        return nullptr;
    }

    const HFS0FileEntry* XCI::GetFileEntryByNcaId(const NcmContentId& ncaId)
    {
        const HFS0FileEntry* fileEntry = nullptr;
        std::string ncaIdStr = tin::util::GetNcaIdString(ncaId);

        if ((fileEntry = this->GetFileEntryByName(ncaIdStr + ".nca")) == nullptr)
        {
            if ((fileEntry = this->GetFileEntryByName(ncaIdStr + ".cnmt.nca")) == nullptr)
            {
                    if ((fileEntry = this->GetFileEntryByName(ncaIdStr + ".ncz")) == nullptr)
                    {
                         if ((fileEntry = this->GetFileEntryByName(ncaIdStr + ".cnmt.ncz")) == nullptr)
                         {
                              return nullptr;
                         }
                    }
            }
        }

        return fileEntry;
    }

    std::vector<const HFS0FileEntry*> XCI::GetFileEntriesByExtension(std::string extension)
    {
        std::vector<const HFS0FileEntry*> entryList;

        for (unsigned int i = 0; i < this->GetSecureHeader()->numFiles; i++)
        {
            const HFS0FileEntry* fileEntry = this->GetFileEntry(i);
            std::string name(this->GetFileEntryName(fileEntry));
            auto foundExtension = name.substr(name.find(".") + 1); 

            if (foundExtension == extension)
                entryList.push_back(fileEntry);
        }

        return entryList;
    }

    const char* XCI::GetFileEntryName(const HFS0FileEntry* fileEntry)
    {
        return hfs0GetFileName(this->GetSecureHeader(), fileEntry);
    }
}