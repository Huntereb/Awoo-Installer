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

#include "install/install.hpp"

#include <switch.h>
#include <cstring>
#include <memory>
#include "util/error.hpp"

#include "nx/ncm.hpp"
#include "util/title_util.hpp"


// TODO: Check NCA files are present
// TODO: Check tik/cert is present
namespace tin::install
{
    Install::Install(NcmStorageId destStorageId, bool ignoreReqFirmVersion) :
        m_destStorageId(destStorageId), m_ignoreReqFirmVersion(ignoreReqFirmVersion), m_contentMeta()
    {
        appletSetMediaPlaybackState(true);
    }

    Install::~Install()
    {
        appletSetMediaPlaybackState(false);
    }

    // TODO: Implement RAII on NcmContentMetaDatabase
    void Install::InstallContentMetaRecords(tin::data::ByteBuffer& installContentMetaBuf, int i)
    {
        NcmContentMetaDatabase contentMetaDatabase;
        NcmContentMetaKey contentMetaKey = m_contentMeta[i].GetContentMetaKey();

        try
        {
            ASSERT_OK(ncmOpenContentMetaDatabase(&contentMetaDatabase, m_destStorageId), "Failed to open content meta database");
            ASSERT_OK(ncmContentMetaDatabaseSet(&contentMetaDatabase, &contentMetaKey, (NcmContentMetaHeader*)installContentMetaBuf.GetData(), installContentMetaBuf.GetSize()), "Failed to set content records");
            ASSERT_OK(ncmContentMetaDatabaseCommit(&contentMetaDatabase), "Failed to commit content records");
        }
        catch (std::runtime_error& e)
        {
            serviceClose(&contentMetaDatabase.s);
            THROW_FORMAT(e.what());
        }

        serviceClose(&contentMetaDatabase.s);
    }

    void Install::InstallApplicationRecord(int i)
    {
        Result rc = 0;
        std::vector<ContentStorageRecord> storageRecords;
        u64 baseTitleId = tin::util::GetBaseTitleId(this->GetTitleId(i), this->GetContentMetaType(i));
        u32 contentMetaCount = 0;

        LOG_DEBUG("Base title Id: 0x%lx", baseTitleId);

        // TODO: Make custom error with result code field
        // 0x410: The record doesn't already exist
        if (R_FAILED(rc = nsCountApplicationContentMeta(baseTitleId, &contentMetaCount)) && rc != 0x410)
        {
            THROW_FORMAT("Failed to count application content meta");
        }
        rc = 0;

        LOG_DEBUG("Content meta count: %u\n", contentMetaCount);

        // Obtain any existing app record content meta and append it to our vector
        if (contentMetaCount > 0)
        {
            storageRecords.resize(contentMetaCount);
            size_t contentStorageBufSize = contentMetaCount * sizeof(ContentStorageRecord);
            auto contentStorageBuf = std::make_unique<ContentStorageRecord[]>(contentMetaCount);
            u32 entriesRead;

            ASSERT_OK(nsListApplicationRecordContentMeta(0, baseTitleId, contentStorageBuf.get(), contentStorageBufSize, &entriesRead), "Failed to list application record content meta");

            if (entriesRead != contentMetaCount)
            {
                THROW_FORMAT("Mismatch between entries read and content meta count");
            }

            memcpy(storageRecords.data(), contentStorageBuf.get(), contentStorageBufSize);
        }

        // Add our new content meta
        ContentStorageRecord storageRecord;
        storageRecord.metaRecord = m_contentMeta[i].GetContentMetaKey();
        storageRecord.storageId = m_destStorageId;
        storageRecords.push_back(storageRecord);

        // Replace the existing application records with our own
        try
        {
            nsDeleteApplicationRecord(baseTitleId);
        }
        catch (...) {}

        LOG_DEBUG("Pushing application record...\n");
        ASSERT_OK(nsPushApplicationRecord(baseTitleId, 0x3, storageRecords.data(), storageRecords.size() * sizeof(ContentStorageRecord)), "Failed to push application record");
    }

    // Validate and obtain all data needed for install
    void Install::Prepare()
    {
        tin::data::ByteBuffer cnmtBuf;

        std::vector<std::tuple<nx::ncm::ContentMeta, NcmContentInfo>> tupelList = this->ReadCNMT();
        
        for (size_t i = 0; i < tupelList.size(); i++) {
            std::tuple<nx::ncm::ContentMeta, NcmContentInfo> cnmtTuple = tupelList[i];
            
            m_contentMeta.push_back(std::get<0>(cnmtTuple));
            NcmContentInfo cnmtContentRecord = std::get<1>(cnmtTuple);

            nx::ncm::ContentStorage contentStorage(m_destStorageId);

            if (!contentStorage.Has(cnmtContentRecord.content_id))
            {
                LOG_DEBUG("Installing CNMT NCA...\n");
                this->InstallNCA(cnmtContentRecord.content_id);
            }
            else
            {
                LOG_DEBUG("CNMT NCA already installed. Proceeding...\n");
            }

            // Parse data and create install content meta
            if (m_ignoreReqFirmVersion)
                LOG_DEBUG("WARNING: Required system firmware version is being IGNORED!\n");

            tin::data::ByteBuffer installContentMetaBuf;
            m_contentMeta[i].GetInstallContentMeta(installContentMetaBuf, cnmtContentRecord, m_ignoreReqFirmVersion);

            this->InstallContentMetaRecords(installContentMetaBuf, i);
            this->InstallApplicationRecord(i);
        }
    }

    void Install::Begin()
    {
        LOG_DEBUG("Installing ticket and cert...\n");
        try
        {
            this->InstallTicketCert();
        }
        catch (std::runtime_error& e)
        {
            LOG_DEBUG("WARNING: Ticket installation failed! This may not be an issue, depending on your use case.\nProceed with caution!\n");
        }

        for (nx::ncm::ContentMeta contentMeta: m_contentMeta) {
            LOG_DEBUG("Installing NCAs...\n");
            for (auto& record : contentMeta.GetContentInfos())
            {
                LOG_DEBUG("Installing from %s\n", tin::util::GetNcaIdString(record.content_id).c_str());
                this->InstallNCA(record.content_id);
            }
        }
    }

    u64 Install::GetTitleId(int i)
    {
        return m_contentMeta[i].GetContentMetaKey().id;
    }

    NcmContentMetaType Install::GetContentMetaType(int i)
    {
        return static_cast<NcmContentMetaType>(m_contentMeta[i].GetContentMetaKey().type);
    }
}
