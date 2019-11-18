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

extern "C"
{
#include <switch/services/fs.h>
}

#include <memory>
#include <tuple>
#include <vector>

#include "install/simple_filesystem.hpp"
#include "data/byte_buffer.hpp"

#include "nx/content_meta.hpp"
#include "nx/ipc/tin_ipc.h"

namespace tin::install
{
    class Install
    {
        protected:
            const NcmStorageId m_destStorageId;
            bool m_ignoreReqFirmVersion = false;
            bool declinedValidation = false;

            nx::ncm::ContentMeta m_contentMeta;

            Install(NcmStorageId destStorageId, bool ignoreReqFirmVersion);
            virtual ~Install();

            virtual std::tuple<nx::ncm::ContentMeta, NcmContentInfo> ReadCNMT() = 0;

            virtual void InstallContentMetaRecords(tin::data::ByteBuffer& installContentMetaBuf);
            virtual void InstallApplicationRecord();
            virtual void InstallTicketCert() = 0;
            virtual void InstallNCA(const NcmContentId &ncaId) = 0;

        public:
            virtual void Prepare();
            virtual void Begin();

            virtual u64 GetTitleId();
            virtual NcmContentMetaType GetContentMetaType();

            virtual void DebugPrintInstallData();
    };
}