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

#include "util/usb_util.hpp"
#include "util/usb_comms_awoo.h"

#include "data/byte_buffer.hpp"
#include "debug.h"
#include "error.hpp"

namespace tin::util
{
    void USBCmdManager::SendCmdHeader(u32 cmdId, size_t dataSize)
    {
        USBCmdHeader header;
        header.magic = 0x30435554; // TUC0 (Tinfoil USB Command 0)
        header.type = USBCmdType::REQUEST;
        header.cmdId = cmdId;
        header.dataSize = dataSize;

        USBWrite(&header, sizeof(USBCmdHeader));
    }

    void USBCmdManager::SendExitCmd()
    {
        USBCmdManager::SendCmdHeader(0, 0);
    }

    USBCmdHeader USBCmdManager::SendFileRangeCmd(std::string nspName, u64 offset, u64 size)
    {
        struct FileRangeCmdHeader
        {
            u64 size;
            u64 offset;
            u64 nspNameLen;
            u64 padding;
        } fRangeHeader;

        fRangeHeader.size = size;
        fRangeHeader.offset = offset;
        fRangeHeader.nspNameLen = nspName.size();
        fRangeHeader.padding = 0;

        USBCmdManager::SendCmdHeader(1, sizeof(FileRangeCmdHeader) + fRangeHeader.nspNameLen);
        USBWrite(&fRangeHeader, sizeof(FileRangeCmdHeader));
        USBWrite(nspName.c_str(), fRangeHeader.nspNameLen);

        USBCmdHeader responseHeader;
        USBRead(&responseHeader, sizeof(USBCmdHeader));
        return responseHeader;
    }

    size_t USBRead(void* out, size_t len, u64 timeout)
    {
        u8* tmpBuf = (u8*)out;
        size_t sizeRemaining = len;
        size_t tmpSizeRead = 0;

        while (sizeRemaining)
        {
            tmpSizeRead = awoo_usbCommsRead(tmpBuf, sizeRemaining, timeout);
            if (tmpSizeRead == 0) return 0;
            tmpBuf += tmpSizeRead;
            sizeRemaining -= tmpSizeRead;
        }

        return len;
    }

    size_t USBWrite(const void* in, size_t len, u64 timeout)
    {
        const u8 *bufptr = (const u8 *)in;
        size_t cursize = len;
        size_t tmpsize = 0;

        while (cursize)
        {
            tmpsize = awoo_usbCommsWrite(bufptr, cursize, timeout);
            if (tmpsize == 0) return 0;
            bufptr += tmpsize;
            cursize -= tmpsize;
        }

        return len;
    }
}