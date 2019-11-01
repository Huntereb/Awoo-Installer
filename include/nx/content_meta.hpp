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

#include <switch/services/ncm.h>
#include <switch/types.h>
#include <vector>

#include "data/byte_buffer.hpp"

namespace nx::ncm
{
    struct PackagedContentInfo
    {
        u8 hash[0x20];
        NcmContentInfo content_info;
    } PACKED;

    struct PackagedContentMetaHeader
    {
        u64 title_id;
        u32 version;
        u8 type;
        u8 _0xd;
        u16 extended_header_size;
        u16 content_count;
        u16 content_meta_count;
        u8 attributes;
        u8 storage_id;
        u8 install_type;
        bool comitted;
        u32 required_system_version;
        u32 _0x1c;
    };

    static_assert(sizeof(PackagedContentMetaHeader) == 0x20, "PackagedContentMetaHeader must be 0x20!");

    class ContentMeta final
    {
        private:
            tin::data::ByteBuffer m_bytes;

        public:
            ContentMeta();
            ContentMeta(u8* data, size_t size);

            PackagedContentMetaHeader GetPackagedContentMetaHeader();
            NcmContentMetaKey GetContentMetaKey();
            std::vector<NcmContentInfo> GetContentInfos();

            void GetInstallContentMeta(tin::data::ByteBuffer& installContentMetaBuffer, NcmContentInfo& cnmtContentInfo, bool ignoreReqFirmVersion);
    };
}