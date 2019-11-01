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

#include <switch/services/ncm.h>

typedef struct {
    u8 c[0x10];
} RightsId;

Result esInitialize();
void esExit();
Service* esGetServiceSession();

Result esImportTicket(void const *tikBuf, size_t tikSize, void const *certBuf, size_t certSize); //1
Result esDeleteTicket(const RightsId *rightsIdBuf, size_t bufSize); //3
Result esGetTitleKey(const RightsId *rightsId, u8 *outBuf, size_t bufSize); //8
Result esCountCommonTicket(u32 *numTickets); //9
Result esCountPersonalizedTicket(u32 *numTickets); // 10
Result esListCommonTicket(u32 *numRightsIdsWritten, RightsId *outBuf, size_t bufSize);
Result esListPersonalizedTicket(u32 *numRightsIdsWritten, RightsId *outBuf, size_t bufSize);
Result esGetCommonTicketData(u64 *unkOut, void *outBuf1, size_t bufSize1, const RightsId* rightsId);