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

#include <switch/types.h>

#define MAGIC_HFS0 0x30534648

namespace tin::install
{
    struct HFS0FileEntry
    {
        u64 dataOffset;
        u64 fileSize;
        u32 stringTableOffset;
        u32 hashedSize;
        u64 padding;
        unsigned char hash[0x20];
    } PACKED;

    static_assert(sizeof(HFS0FileEntry) == 0x40, "HFS0FileEntry must be 0x18");

    struct HFS0BaseHeader
    {
        u32 magic;
        u32 numFiles;
        u32 stringTableSize;
        u32 reserved;
    } PACKED;

    static_assert(sizeof(HFS0BaseHeader) == 0x10, "HFS0BaseHeader must be 0x10");

    NX_CONSTEXPR const HFS0FileEntry *hfs0GetFileEntry(const HFS0BaseHeader *header, u32 i)
    {
        if (i >= header->numFiles)
            return NULL;
        return (const HFS0FileEntry*)(header + 0x1 + i * 0x4);
    }

    NX_CONSTEXPR const char *hfs0GetStringTable(const HFS0BaseHeader *header)
    {
        return (const char*)(header + 0x1 + header->numFiles * 0x4);
    }

    NX_CONSTEXPR u64 hfs0GetHeaderSize(const HFS0BaseHeader *header)
    {
        return 0x1 + header->numFiles * 0x4 + header->stringTableSize;
    }

    NX_CONSTEXPR const char *hfs0GetFileName(const HFS0BaseHeader *header, u32 i)
    {
        return hfs0GetStringTable(header) + hfs0GetFileEntry(header, i)->stringTableOffset;
    }

    NX_CONSTEXPR const char *hfs0GetFileName(const HFS0BaseHeader *header, const HFS0FileEntry *entry)
    {
        return hfs0GetStringTable(header) + entry->stringTableOffset;
    }
}