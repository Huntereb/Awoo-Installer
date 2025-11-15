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

#include <switch.h>
#include <string>

namespace tin::util
{
    enum USBCmdType : u8
    {
        REQUEST = 0,
        RESPONSE = 1
    };

    struct USBCmdHeader
    {
        u32 magic;
        USBCmdType type;
        u8 padding[0x3] = {0};
        u32 cmdId;
        u64 dataSize;
        u8 reserved[0xC] = {0};
    } NX_PACKED;

    static_assert(sizeof(USBCmdHeader) == 0x20, "USBCmdHeader must be 0x20!");

    class USBCmdManager
    {
        public:
            static void SendCmdHeader(u32 cmdId, size_t dataSize);

            static void SendExitCmd();
            static USBCmdHeader SendFileRangeCmd(std::string nspName, u64 offset, u64 size);
    };

    size_t USBRead(void* out, size_t len, u64 timeout = 5000000000);
    size_t USBWrite(const void* in, size_t len, u64 timeout = 5000000000);
}