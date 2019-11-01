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
#include <switch.h>

typedef struct ServiceGuard {
    Mutex mutex;
    u32 refCount;
} ServiceGuard;

NX_INLINE bool serviceGuardBeginInit(ServiceGuard* g)
{
    mutexLock(&g->mutex);
    return (g->refCount++) == 0;
}

NX_INLINE Result serviceGuardEndInit(ServiceGuard* g, Result rc, void (*cleanupFunc)(void))
{
    if (R_FAILED(rc)) {
        cleanupFunc();
        --g->refCount;
    }
    mutexUnlock(&g->mutex);
    return rc;
}

NX_INLINE void serviceGuardExit(ServiceGuard* g, void (*cleanupFunc)(void))
{
    mutexLock(&g->mutex);
    if (g->refCount && (--g->refCount) == 0)
        cleanupFunc();
    mutexUnlock(&g->mutex);
}

#define NX_GENERATE_SERVICE_GUARD_PARAMS(name, _paramdecl, _parampass) \
\
static ServiceGuard g_##name##Guard; \
NX_INLINE Result _##name##Initialize _paramdecl; \
static void _##name##Cleanup(void); \
\
Result name##Initialize _paramdecl \
{ \
    Result rc = 0; \
    if (serviceGuardBeginInit(&g_##name##Guard)) \
        rc = _##name##Initialize _parampass; \
    return serviceGuardEndInit(&g_##name##Guard, rc, _##name##Cleanup); \
} \
\
void name##Exit(void) \
{ \
    serviceGuardExit(&g_##name##Guard, _##name##Cleanup); \
}

#define NX_GENERATE_SERVICE_GUARD(name) NX_GENERATE_SERVICE_GUARD_PARAMS(name, (void), ())
