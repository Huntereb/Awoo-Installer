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

#include "install/install_xci.hpp"
#include "util/file_util.hpp"
#include "util/title_util.hpp"
#include "nx/nca_writer.h"
#include "util/debug.h"
#include "util/error.hpp"
#include "nspInstall.hpp"
#include "ui/MainApplication.hpp"

namespace tin::install::xci
{
    XCIInstallTask::XCIInstallTask(tin::install::xci::XCI& xci, NcmStorageId destStorageId, bool ignoreReqFirmVersion) :
        Install(destStorageId, ignoreReqFirmVersion), m_xci(&xci)
    {
        m_xci->RetrieveHeader();
    }

    std::vector<std::tuple<nx::ncm::ContentMeta, NcmContentInfo>> XCIInstallTask::ReadCNMT()
    {
        std::vector<std::tuple<nx::ncm::ContentMeta, NcmContentInfo>> CNMTList;

        for (const HFS0FileEntry* fileEntry : m_xci->GetFileEntriesByExtension("cnmt.nca")) {
            if (fileEntry == nullptr)
                THROW_FORMAT("Failed to find cnmt file entry!\n");

            std::string cnmtNcaName(m_xci->GetFileEntryName(fileEntry));
            NcmContentId cnmtContentId = tin::util::GetNcaIdFromString(cnmtNcaName);
            size_t cnmtNcaSize = fileEntry->fileSize;

            nx::ncm::ContentStorage contentStorage(m_destStorageId);

            printf("CNMT Name: %s\n", cnmtNcaName.c_str());

            // We install the cnmt nca early to read from it later
            this->InstallNCA(cnmtContentId);
            std::string cnmtNCAFullPath = contentStorage.GetPath(cnmtContentId);

            NcmContentInfo cnmtContentInfo;
            cnmtContentInfo.content_id = cnmtContentId;
            *(u64*)&cnmtContentInfo.size = cnmtNcaSize & 0xFFFFFFFFFFFF;
            cnmtContentInfo.content_type = NcmContentType_Meta;

            CNMTList.push_back( { tin::util::GetContentMetaFromNCA(cnmtNCAFullPath), cnmtContentInfo } );
        }
        
        return CNMTList;
    }

    void XCIInstallTask::InstallNCA(const NcmContentId& ncaId)
    {
        const HFS0FileEntry* fileEntry = m_xci->GetFileEntryByNcaId(ncaId);
        std::string ncaFileName = m_xci->GetFileEntryName(fileEntry);
        size_t ncaSize = fileEntry->fileSize;

        printf("Installing %s to storage Id %u\n", ncaFileName.c_str(), m_destStorageId);

        std::shared_ptr<nx::ncm::ContentStorage> contentStorage(new nx::ncm::ContentStorage(m_destStorageId));

        // Attempt to delete any leftover placeholders
        try
        {
            contentStorage->DeletePlaceholder(*(NcmPlaceHolderId*)&ncaId);
        }
        catch (...) {}

        printf("Size: 0x%lx\n", ncaSize);

        if (m_xci->CanStream()) {
            m_xci->StreamToPlaceholder(contentStorage, ncaId);
        } else {
            NcaWriter writer(ncaId, contentStorage);

            float progress;

            u64 fileStart = m_xci->GetDataOffset() + fileEntry->dataOffset;
            u64 fileOff = 0;
            size_t readSize = 0x400000; // 4MB buff
            auto readBuffer = std::make_unique<u8[]>(readSize);

            try
            {
                inst::ui::setInstInfoText("Installing " + ncaFileName + "...");
                inst::ui::setInstBarPerc(0);
                while (fileOff < ncaSize)
                {
                    progress = (float) fileOff / (float) ncaSize;

                    if (fileOff % (0x400000 * 3) == 0) {
                        printf("> Progress: %lu/%lu MB (%d%s)\r", (fileOff / 1000000), (ncaSize / 1000000), (int)(progress * 100.0), "%");
                        inst::ui::setInstBarPerc((double)(progress * 100.0));
                    }

                    if (fileOff + readSize >= ncaSize) readSize = ncaSize - fileOff;

                    m_xci->BufferData(readBuffer.get(), fileOff + fileStart, readSize);
                    writer.write(readBuffer.get(), readSize);

                    fileOff += readSize;
                }
                inst::ui::setInstBarPerc(100);
            }
            catch (std::exception& e)
            {
                printf("something went wrong: %s\n", e.what());
            }

            writer.close();
        }

        // Clean up the line for whatever comes next
        printf("                                                           \r");
        printf("Registering placeholder...\n");

        try
        {
            contentStorage->Register(*(NcmPlaceHolderId*)&ncaId, ncaId);
        }
        catch (...)
        {
            printf(("Failed to register " + ncaFileName + ". It may already exist.\n").c_str());
        }

        try
        {
            contentStorage->DeletePlaceholder(*(NcmPlaceHolderId*)&ncaId);
        }
        catch (...) {}
    }

    void XCIInstallTask::InstallTicketCert()
    {
        // Read the tik files and put it into a buffer
        std::vector<const HFS0FileEntry*> tikFileEntries = m_xci->GetFileEntriesByExtension("tik");
        std::vector<const HFS0FileEntry*> certFileEntries = m_xci->GetFileEntriesByExtension("cert");

        for (size_t i = 0; i < tikFileEntries.size(); i++)
        {
            if (tikFileEntries[i] == nullptr)
            {
                printf("Remote tik file is missing.\n");
                throw std::runtime_error("Remote tik file is not present!");
            }

            u64 tikSize = tikFileEntries[i]->fileSize;
            auto tikBuf = std::make_unique<u8[]>(tikSize);
            printf("> Reading tik\n");
            m_xci->BufferData(tikBuf.get(), m_xci->GetDataOffset() + tikFileEntries[i]->dataOffset, tikSize);

            if (certFileEntries[i] == nullptr)
            {
                printf("Remote cert file is missing.\n");
                throw std::runtime_error("Remote cert file is not present!");
            }

            u64 certSize = certFileEntries[i]->fileSize;
            auto certBuf = std::make_unique<u8[]>(certSize);
            printf("> Reading cert\n");
            m_xci->BufferData(certBuf.get(), m_xci->GetDataOffset() + certFileEntries[i]->dataOffset, certSize);

            // Finally, let's actually import the ticket
            ASSERT_OK(esImportTicket(tikBuf.get(), tikSize, certBuf.get(), certSize), "Failed to import ticket");
        }
    }
}