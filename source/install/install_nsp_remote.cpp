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

#include "install/install_nsp_remote.hpp"

#include <machine/endian.h>
#include "install/nca.hpp"
#include "nx/fs.hpp"
#include "nx/ncm.hpp"
#include "util/config.hpp"
#include "util/crypto.hpp"
#include "util/file_util.hpp"
#include "util/title_util.hpp"
#include "util/debug.h"
#include "util/error.hpp"
#include "ui/MainApplication.hpp"

namespace inst::ui {
     extern MainApplication *mainApp;
}

namespace tin::install::nsp
{
    RemoteNSPInstall::RemoteNSPInstall(NcmStorageId destStorageId, bool ignoreReqFirmVersion, RemoteNSP* remoteNSP) :
        Install(destStorageId, ignoreReqFirmVersion), m_remoteNSP(remoteNSP)
    {
        m_remoteNSP->RetrieveHeader();
    }

    std::vector<std::tuple<nx::ncm::ContentMeta, NcmContentInfo>> RemoteNSPInstall::ReadCNMT()
    {
        const PFS0FileEntry* fileEntry = m_remoteNSP->GetFileEntryByExtension("cnmt.nca");

        if (fileEntry == nullptr)
            THROW_FORMAT("Failed to find cnmt file entry!\n");

        std::string cnmtNcaName(m_remoteNSP->GetFileEntryName(fileEntry));
        NcmContentId cnmtContentId = tin::util::GetNcaIdFromString(cnmtNcaName);
        size_t cnmtNcaSize = fileEntry->fileSize;

        nx::ncm::ContentStorage contentStorage(m_destStorageId);

        LOG_DEBUG("CNMT Name: %s\n", cnmtNcaName.c_str());

        // We install the cnmt nca early to read from it later
        this->InstallNCA(cnmtContentId);
        std::string cnmtNCAFullPath = contentStorage.GetPath(cnmtContentId);

        NcmContentInfo cnmtContentInfo;
        cnmtContentInfo.content_id = cnmtContentId;
        *(u64*)&cnmtContentInfo.size = cnmtNcaSize & 0xFFFFFFFFFFFF;
        cnmtContentInfo.content_type = NcmContentType_Meta;

        return { { tin::util::GetContentMetaFromNCA(cnmtNCAFullPath), cnmtContentInfo } };
    }

    void RemoteNSPInstall::InstallNCA(const NcmContentId& ncaId)
    {
        const PFS0FileEntry* fileEntry = m_remoteNSP->GetFileEntryByNcaId(ncaId);
        std::string ncaFileName = m_remoteNSP->GetFileEntryName(fileEntry);

        #ifdef NXLINK_DEBUG
        size_t ncaSize = fileEntry->fileSize;
        LOG_DEBUG("Installing %s to storage Id %u\n", ncaFileName.c_str(), m_destStorageId);
        #endif

        std::shared_ptr<nx::ncm::ContentStorage> contentStorage(new nx::ncm::ContentStorage(m_destStorageId));

        // Attempt to delete any leftover placeholders
        try
        {
            contentStorage->DeletePlaceholder(*(NcmPlaceHolderId*)&ncaId);
        }
        catch (...) {}

        LOG_DEBUG("Size: 0x%lx\n", ncaSize);

        if (inst::config::validateNCAs && !declinedValidation)
        {
            tin::install::NcaHeader* header = new NcaHeader;
            m_remoteNSP->BufferData(header, m_remoteNSP->GetDataOffset() + fileEntry->dataOffset, sizeof(tin::install::NcaHeader));

            Crypto::AesXtr crypto(Crypto::Keys().headerKey, false);
            crypto.decrypt(header, header, sizeof(tin::install::NcaHeader), 0, 0x200);

            if (header->magic != MAGIC_NCA3)
                THROW_FORMAT("Invalid NCA magic");

            if (!Crypto::rsa2048PssVerify(&header->magic, 0x200, header->fixed_key_sig, Crypto::NCAHeaderSignature))
            {
                int rc = inst::ui::mainApp->CreateShowDialog("Invalid NCA signature detected!", "Improperly signed software should only be installed from trustworthy\nsources. Files containing cartridge repacks and DLC unlockers will always\nshow this warning. You can disable this check in Awoo Installer's settings.\n\nAre you sure you want to continue the installation?", {"Cancel", "Yes, I understand the risks"}, false);
                if (rc != 1)
                    THROW_FORMAT(("The requested NCA (" + tin::util::GetNcaIdString(ncaId) + ") is not properly signed").c_str());
                declinedValidation = true;
            }
        }

        m_remoteNSP->StreamToPlaceholder(contentStorage, ncaId);

        LOG_DEBUG("Registering placeholder...\n");

        try
        {
            contentStorage->Register(*(NcmPlaceHolderId*)&ncaId, ncaId);
        }
        catch (...)
        {
            LOG_DEBUG(("Failed to register " + ncaFileName + ". It may already exist.\n").c_str());
        }

        try
        {
            contentStorage->DeletePlaceholder(*(NcmPlaceHolderId*)&ncaId);
        }
        catch (...) {}
    }

    void RemoteNSPInstall::InstallTicketCert()
    {
        // Read the tik file and put it into a buffer
        const PFS0FileEntry* tikFileEntry = m_remoteNSP->GetFileEntryByExtension("tik");

        if (tikFileEntry == nullptr)
        {
            LOG_DEBUG("Remote tik file is missing.\n");
            throw std::runtime_error("Remote tik file is not present!");
        }

        u64 tikSize = tikFileEntry->fileSize;
        auto tikBuf = std::make_unique<u8[]>(tikSize);
        LOG_DEBUG("> Reading tik\n");
        m_remoteNSP->BufferData(tikBuf.get(), m_remoteNSP->GetDataOffset() + tikFileEntry->dataOffset, tikSize);

        // Read the cert file and put it into a buffer
        const PFS0FileEntry* certFileEntry = m_remoteNSP->GetFileEntryByExtension("cert");

        if (certFileEntry == nullptr)
        {
            LOG_DEBUG("Remote cert file is missing.\n");
            throw std::runtime_error("Remote cert file is not present!");
        }

        u64 certSize = certFileEntry->fileSize;
        auto certBuf = std::make_unique<u8[]>(certSize);
        LOG_DEBUG("> Reading cert\n");
        m_remoteNSP->BufferData(certBuf.get(), m_remoteNSP->GetDataOffset() + certFileEntry->dataOffset, certSize);

        // Finally, let's actually import the ticket
        ASSERT_OK(esImportTicket(tikBuf.get(), tikSize, certBuf.get(), certSize), "Failed to import ticket");
    }
}