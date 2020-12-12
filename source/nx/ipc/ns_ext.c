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

#include "nx/ipc/ns_ext.h"

#include <switch.h>

Service g_nsAppManSrv;

Result nsextInitialize(void) {
    Result rc = nsInitialize();

    if (R_SUCCEEDED(rc)) {
        if(hosversionBefore(3,0,0)) {
            g_nsAppManSrv = *nsGetServiceSession_ApplicationManagerInterface();
        } else {
            rc = nsGetApplicationManagerInterface(&g_nsAppManSrv);
        }
    }

    return rc;
}

void nsextExit(void) {
    if(hosversionAtLeast(3,0,0))
        serviceClose(&g_nsAppManSrv);
    nsExit();
}

Result nsPushApplicationRecord(u64 application_id, NsApplicationRecordType last_modified_event, ContentStorageRecord *content_records, u32 count) {
    struct {
        u8 last_modified_event;
        u64 application_id;
    } in = { last_modified_event, application_id };
    
    return serviceDispatchIn(&g_nsAppManSrv, 16, in,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
        .buffers = { { content_records, count * sizeof(*content_records) }
    });
}
