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

namespace tin::install
{
    struct PFS0FileEntry
    {
        u64 dataOffset;
        u64 fileSize;
        u32 stringTableOffset;
        u32 padding;
    } __attribute__((packed));

    static_assert(sizeof(PFS0FileEntry) == 0x18, "PFS0FileEntry must be 0x18");

    struct PFS0BaseHeader
    {
        u32 magic;
        u32 numFiles;
        u32 stringTableSize;
        u32 reserved;
    } __attribute__((packed));

    static_assert(sizeof(PFS0BaseHeader) == 0x10, "PFS0BaseHeader must be 0x10");
}
