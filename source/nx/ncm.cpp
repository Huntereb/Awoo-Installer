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

#include "nx/ncm.hpp"
#include "util/error.hpp"

namespace nx::ncm
{
    ContentStorage::ContentStorage(FsStorageId storageId) 
    {
        ASSERT_OK(ncmOpenContentStorage(&m_contentStorage, storageId), "Failed to open NCM ContentStorage");
    }

    ContentStorage::~ContentStorage()
    {
        serviceClose(&m_contentStorage.s);
    }

    void ContentStorage::CreatePlaceholder(const NcmContentId &placeholderId, const NcmContentId &registeredId, size_t size)
    {
        ASSERT_OK(ncmContentStorageCreatePlaceHolder(&m_contentStorage, &placeholderId, &registeredId, size), "Failed to create placeholder");
    }

    void ContentStorage::DeletePlaceholder(const NcmContentId &placeholderId)
    {
        ASSERT_OK(ncmContentStorageDeletePlaceHolder(&m_contentStorage, &placeholderId), "Failed to delete placeholder");
    }

    void ContentStorage::WritePlaceholder(const NcmContentId &placeholderId, u64 offset, void *buffer, size_t bufSize)
    {
        ASSERT_OK(ncmContentStorageWritePlaceHolder(&m_contentStorage, &placeholderId, offset, buffer, bufSize), "Failed to write to placeholder");
    }

    void ContentStorage::Register(const NcmContentId &placeholderId, const NcmContentId &registeredId)
    {
        ASSERT_OK(ncmContentStorageRegister(&m_contentStorage, &registeredId, &placeholderId), "Failed to register placeholder NCA");
    }

    void ContentStorage::Delete(const NcmContentId &registeredId)
    {
        ASSERT_OK(ncmContentStorageDelete(&m_contentStorage, &registeredId), "Failed to delete registered NCA");
    }

    bool ContentStorage::Has(const NcmContentId &registeredId)
    {
        bool hasNCA = false;
        ASSERT_OK(ncmContentStorageHas(&m_contentStorage, &hasNCA, &registeredId), "Failed to check if NCA is present");
        return hasNCA;
    }

    std::string ContentStorage::GetPath(const NcmContentId &registeredId)
    {
        char pathBuf[FS_MAX_PATH] = {0};
        ASSERT_OK(ncmContentStorageGetPath(&m_contentStorage, pathBuf, FS_MAX_PATH, &registeredId), "Failed to get installed NCA path");
        return std::string(pathBuf);
    }
}