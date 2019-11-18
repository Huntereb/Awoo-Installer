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

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <memory>
#include <string>
#include <machine/endian.h>

#include "nx/ncm.hpp"
#include "install/nca.hpp"
#include "util/config.hpp"
#include "util/crypto.hpp"
#include "nx/nca_writer.h"
#include "util/debug.h"
#include "util/error.hpp"
#include "util/file_util.hpp"
#include "util/title_util.hpp"
#include "nspInstall.hpp"
#include "ui/MainApplication.hpp"

namespace inst::ui {
    extern MainApplication *mainApp;
}

namespace tin::install::nsp
{
    NSPInstallTask::NSPInstallTask(tin::install::nsp::SimpleFileSystem& simpleFileSystem, NcmStorageId destStorageId, bool ignoreReqFirmVersion) :
        Install(destStorageId, ignoreReqFirmVersion), m_simpleFileSystem(&simpleFileSystem)
    {

    }

    std::tuple<nx::ncm::ContentMeta, NcmContentInfo> NSPInstallTask::ReadCNMT()
    {
        NcmContentInfo cnmtRecord = tin::util::CreateNSPCNMTContentRecord(this->m_simpleFileSystem->m_absoluteRootPath.substr(0, this->m_simpleFileSystem->m_absoluteRootPath.size() - 1));
        nx::ncm::ContentStorage contentStorage(m_destStorageId);
        this->InstallNCA(cnmtRecord.content_id);
        std::string cnmtNCAFullPath = contentStorage.GetPath(cnmtRecord.content_id);
        return { tin::util::GetContentMetaFromNCA(cnmtNCAFullPath), cnmtRecord };
    }

    void NSPInstallTask::InstallTicketCert()
    {
        // Read the tik file and put it into a buffer
        auto tikName = m_simpleFileSystem->GetFileNameFromExtension("", "tik");
        printf("> Getting tik size\n");
        auto tikFile = m_simpleFileSystem->OpenFile(tikName);
        u64 tikSize = tikFile.GetSize();
        auto tikBuf = std::make_unique<u8[]>(tikSize);
        printf("> Reading tik\n");
        tikFile.Read(0x0, tikBuf.get(), tikSize);

        // Read the cert file and put it into a buffer
        auto certName = m_simpleFileSystem->GetFileNameFromExtension("", "cert");
        printf("> Getting cert size\n");
        auto certFile = m_simpleFileSystem->OpenFile(certName);
        u64 certSize = certFile.GetSize();
        auto certBuf = std::make_unique<u8[]>(certSize);
        printf("> Reading cert\n");
        certFile.Read(0x0, certBuf.get(), certSize);

        // Finally, let's actually import the ticket
        ASSERT_OK(esImportTicket(tikBuf.get(), tikSize, certBuf.get(), certSize), "Failed to import ticket");
        //consoleUpdate(NULL);
    }

    void NSPInstallTask::InstallNCA(const NcmContentId &ncaId)
    {
        std::string ncaName = tin::util::GetNcaIdString(ncaId);

        if (m_simpleFileSystem->HasFile(ncaName + ".nca"))
            ncaName += ".nca";
        else if (m_simpleFileSystem->HasFile(ncaName + ".cnmt.nca"))
            ncaName += ".cnmt.nca";
          else if (m_simpleFileSystem->HasFile(ncaName + ".ncz"))
            ncaName += ".ncz";
        else if (m_simpleFileSystem->HasFile(ncaName + ".cnmt.ncz"))
            ncaName += ".cnmt.ncz";
        else
        {
            throw std::runtime_error(("Failed to find NCA file " + ncaName + ".nca/.cnmt.nca").c_str());
        }

        printf("NcaId: %s\n", ncaName.c_str());
        printf("Dest storage Id: %u\n", m_destStorageId);

        std::shared_ptr<nx::ncm::ContentStorage> contentStorage(new nx::ncm::ContentStorage(m_destStorageId));

        // Attempt to delete any leftover placeholders
        try
        {
            contentStorage->DeletePlaceholder(*(NcmPlaceHolderId*)&ncaId);
        }
        catch (...) {}

        auto ncaFile = m_simpleFileSystem->OpenFile(ncaName);

        if (inst::config::validateNCAs && !declinedValidation)
        {
            tin::install::NcaHeader header;
            ncaFile.Read(0, &header, 0xc00);
            Crypto::AesXtr crypto(Crypto::Keys().headerKey);
            crypto.decrypt(&header, &header, sizeof(header), 0, 0x200);

            if (header.magic != MAGIC_NCA3)
                THROW_FORMAT("Invalid NCA magic");

            if (!Crypto::rsa2048PssVerify(&header.magic, 0x200, header.fixed_key_sig, Crypto::NCAHeaderSignature))
            {
                int rc = inst::ui::mainApp->CreateShowDialog("Invalid NCA signature detected!", "The software you are trying to install may contain malicious contents!\nOnly install improperly signed software from trustworthy sources!\nThis warning can be disabled in Awoo Installer's settings.\n\nAre you sure you want to continue the installation?", {"Cancel", "Yes, I want a brick"}, false);
                if (rc != 1)
                    THROW_FORMAT(("The requested NCA (" + tin::util::GetNcaIdString(ncaId) + ") is not properly signed").c_str());
                declinedValidation = true;
            }
        }

        size_t ncaSize = ncaFile.GetSize();
        u64 fileOff = 0;
        size_t readSize = 0x400000; // 4MB buff
        auto readBuffer = std::make_unique<u8[]>(readSize);

        if (readBuffer == NULL) 
            throw std::runtime_error(("Failed to allocate read buffer for " + ncaName).c_str());

        printf("Size: 0x%lx\n", ncaSize);

        NcaWriter writer(ncaId, contentStorage);

        float progress;
        bool failed = false;

        //consoleUpdate(NULL);

        try
        {
            inst::ui::setInstInfoText("Installing " + ncaName + "...");
            inst::ui::setInstBarPerc(0);
            while (fileOff < ncaSize) 
            {   
                // Clear the buffer before we read anything, just to be sure    
                progress = (float)fileOff / (float)ncaSize;

                if (fileOff % (0x400000 * 3) == 0) {
                    printf("> Progress: %lu/%lu MB (%d%s)\r", (fileOff / 1000000), (ncaSize / 1000000), (int)(progress * 100.0), "%");
                    inst::ui::setInstBarPerc((double)(progress * 100.0));
                }

                if (fileOff + readSize >= ncaSize) readSize = ncaSize - fileOff;

                ncaFile.Read(fileOff, readBuffer.get(), readSize);
                writer.write(readBuffer.get(), readSize);

                fileOff += readSize;
                //consoleUpdate(NULL);
            }
            inst::ui::setInstBarPerc(100);
        }
        catch (...)
        {
            failed = true;
        }

        writer.close();

        // Clean up the line for whatever comes next
        printf("                                                           \r");
        printf("Registering placeholder...\n");

        if (!failed)
        {
            try
            {
                contentStorage->Register(*(NcmPlaceHolderId*)&ncaId, ncaId);
            }
            catch (...)
            {
                printf(("Failed to register " + ncaName + ". It may already exist.\n").c_str());
            }
        }

        try
        {
            contentStorage->DeletePlaceholder(*(NcmPlaceHolderId*)&ncaId);
        }
        catch (...) {}
    }
}