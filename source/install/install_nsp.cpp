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

#include "install/install_nsp.hpp"

#include <machine/endian.h>
#include <thread>

#include "install/nca.hpp"
#include "nx/fs.hpp"
#include "nx/ncm.hpp"
#include "util/config.hpp"
#include "util/crypto.hpp"
#include "util/file_util.hpp"
#include "util/title_util.hpp"
#include "util/debug.h"
#include "util/error.hpp"
#include "util/util.hpp"
#include "util/lang.hpp"
#include "ui/MainApplication.hpp"

namespace inst::ui {
     extern MainApplication *mainApp;
}

namespace tin::install::nsp
{
    NSPInstall::NSPInstall(NcmStorageId destStorageId, bool ignoreReqFirmVersion, const std::shared_ptr<NSP>& remoteNSP) :
        Install(destStorageId, ignoreReqFirmVersion), m_NSP(remoteNSP)
    {
        m_NSP->RetrieveHeader();
    }

    std::vector<std::tuple<nx::ncm::ContentMeta, NcmContentInfo>> NSPInstall::ReadCNMT()
    {
        std::vector<std::tuple<nx::ncm::ContentMeta, NcmContentInfo>> CNMTList;

        for (const PFS0FileEntry* fileEntry : m_NSP->GetFileEntriesByExtension("cnmt.nca")) {
            std::string cnmtNcaName(m_NSP->GetFileEntryName(fileEntry));
            NcmContentId cnmtContentId = tin::util::GetNcaIdFromString(cnmtNcaName);
            size_t cnmtNcaSize = fileEntry->fileSize;

            nx::ncm::ContentStorage contentStorage(m_destStorageId);

            LOG_DEBUG("CNMT Name: %s\n", cnmtNcaName.c_str());

            // We install the cnmt nca early to read from it later
            this->InstallNCA(cnmtContentId);
            std::string cnmtNCAFullPath = contentStorage.GetPath(cnmtContentId);

            NcmContentInfo cnmtContentInfo;
            cnmtContentInfo.content_id = cnmtContentId;
            ncmU64ToContentInfoSize(cnmtNcaSize & 0xFFFFFFFFFFFF, &cnmtContentInfo);
            cnmtContentInfo.content_type = NcmContentType_Meta;

            CNMTList.push_back( { tin::util::GetContentMetaFromNCA(cnmtNCAFullPath), cnmtContentInfo } );
        }

        return CNMTList;
    }

    void NSPInstall::InstallNCA(const NcmContentId& ncaId)
    {
        const PFS0FileEntry* fileEntry = m_NSP->GetFileEntryByNcaId(ncaId);
        std::string ncaFileName = m_NSP->GetFileEntryName(fileEntry);

        #ifdef NXLINK_DEBUG
        size_t ncaSize = fileEntry->fileSize;
        LOG_DEBUG("Installing %s to storage Id %u\n", ncaFileName.c_str(), m_destStorageId);
        #endif

        std::shared_ptr<nx::ncm::ContentStorage> contentStorage(new nx::ncm::ContentStorage(m_destStorageId));

        // Attempt to delete any leftover placeholders
        try {
            contentStorage->DeletePlaceholder(*(NcmPlaceHolderId*)&ncaId);
        }
        catch (...) {}

        LOG_DEBUG("Size: 0x%lx\n", ncaSize);

        if (inst::config::validateNCAs && !m_declinedValidation)
        {
            tin::install::NcaHeader* header = new NcaHeader;
            m_NSP->BufferData(header, m_NSP->GetDataOffset() + fileEntry->dataOffset, sizeof(tin::install::NcaHeader));

            Crypto::AesXtr crypto(Crypto::Keys().headerKey, false);
            crypto.decrypt(header, header, sizeof(tin::install::NcaHeader), 0, 0x200);

            if (header->magic != MAGIC_NCA3)
                THROW_FORMAT("Invalid NCA magic");

            if (!Crypto::rsa2048PssVerify(&header->magic, 0x200, header->fixed_key_sig, Crypto::NCAHeaderSignature))
            {
                std::string audioPath = "romfs:/audio/bark.wav";
                if (inst::config::gayMode) audioPath = "";
                if (std::filesystem::exists(inst::config::appDir + "/bark.wav")) audioPath = inst::config::appDir + "/bark.wav";
                std::thread audioThread(inst::util::playAudio,audioPath);
                int rc = inst::ui::mainApp->CreateShowDialog("inst.nca_verify.title"_lang, "inst.nca_verify.desc"_lang, {"common.cancel"_lang, "inst.nca_verify.opt1"_lang}, false);
                audioThread.join();
                if (rc != 1)
                    THROW_FORMAT(("inst.nca_verify.error"_lang + tin::util::GetNcaIdString(ncaId)).c_str());
                m_declinedValidation = true;
            }
            delete header;
        }

        m_NSP->StreamToPlaceholder(contentStorage, ncaId);

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

    void NSPInstall::InstallTicketCert()
    {
        // Read the tik files and put it into a buffer
        std::vector<const PFS0FileEntry*> tikFileEntries = m_NSP->GetFileEntriesByExtension("tik");
        std::vector<const PFS0FileEntry*> certFileEntries = m_NSP->GetFileEntriesByExtension("cert");

        for (size_t i = 0; i < tikFileEntries.size(); i++)
        {
            if (tikFileEntries[i] == nullptr) {
                LOG_DEBUG("Remote tik file is missing.\n");
                THROW_FORMAT("Remote tik file is not present!");
            }

            u64 tikSize = tikFileEntries[i]->fileSize;
            auto tikBuf = std::make_unique<u8[]>(tikSize);
            LOG_DEBUG("> Reading tik\n");
            m_NSP->BufferData(tikBuf.get(), m_NSP->GetDataOffset() + tikFileEntries[i]->dataOffset, tikSize);

            if (certFileEntries[i] == nullptr)
            {
                LOG_DEBUG("Remote cert file is missing.\n");
                THROW_FORMAT("Remote cert file is not present!");
            }

            u64 certSize = certFileEntries[i]->fileSize;
            auto certBuf = std::make_unique<u8[]>(certSize);
            LOG_DEBUG("> Reading cert\n");
            m_NSP->BufferData(certBuf.get(), m_NSP->GetDataOffset() + certFileEntries[i]->dataOffset, certSize);

            // Finally, let's actually import the ticket
            ASSERT_OK(esImportTicket(tikBuf.get(), tikSize, certBuf.get(), certSize), "Failed to import ticket");
        }
    }
}