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

#include <switch/services/ns.h>
#include <switch/services/ncm.h>

typedef struct {
    u64 titleID;
    u64 unk;
    u64 size;
} PACKED ApplicationRecord;

typedef struct {
    NcmContentMetaKey metaRecord;
    u64 storageId;
} PACKED ContentStorageRecord;

Result nsextInitialize(void);
void nsextExit(void);

Result nsPushApplicationRecord(u64 title_id, u8 last_modified_event, ContentStorageRecord *content_records_buf, size_t buf_size);
Result nsListApplicationRecordContentMeta(u64 offset, u64 titleID, void *out_buf, size_t out_buf_size, u32 *entries_read_out);
Result nsDeleteApplicationRecord(u64 titleID);
Result nsLaunchApplication(u64 titleID);
Result nsPushLaunchVersion(u64 titleID, u32 version);
Result nsDisableApplicationAutoUpdate(u64 titleID);
Result nsGetContentMetaStorage(const NcmContentMetaKey *record, u8 *out);
Result nsBeginInstallApplication(u64 tid, u32 unk, u8 storageId);
Result nsInvalidateAllApplicationControlCache(void);
Result nsInvalidateApplicationControlCache(u64 tid);
Result nsCheckApplicationLaunchRights(u64 tid);
Result nsGetApplicationContentPath(u64 titleId, u8 type, char *outBuf, size_t bufSize);
