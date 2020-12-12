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

typedef enum {
    NsApplicationRecordType_Installed       = 0x3,
    NsApplicationRecordType_GamecardMissing = 0x5,
    NsApplicationRecordType_Archived        = 0xB,
} NsApplicationRecordType;

typedef struct {
    NcmContentMetaKey metaRecord;
    u64 storageId;
} ContentStorageRecord;

Result nsextInitialize(void);
void nsextExit(void);

Result nsPushApplicationRecord(u64 application_id, NsApplicationRecordType last_modified_event, ContentStorageRecord *content_records, u32 count);
