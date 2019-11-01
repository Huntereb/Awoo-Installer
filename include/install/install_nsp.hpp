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
#include "install/install.hpp"
#include "install/simple_filesystem.hpp"
#include "nx/content_meta.hpp"
#include "nx/ipc/tin_ipc.h"

namespace tin::install::nsp
{
    class NSPInstallTask : public Install
    {
        private:
            tin::install::nsp::SimpleFileSystem* const m_simpleFileSystem;

        protected:
            std::tuple<nx::ncm::ContentMeta, NcmContentInfo> ReadCNMT() override;
            void InstallNCA(const NcmContentId& ncaId) override;
            void InstallTicketCert() override;

        public:
            NSPInstallTask(tin::install::nsp::SimpleFileSystem& simpleFileSystem, FsStorageId destStorageId, bool ignoreReqFirmVersion);
    };
};

