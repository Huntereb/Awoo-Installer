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

#include "util/debug.h"

#include <string.h>

#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <switch/runtime/nxlink.h>

#ifdef NXLINK_DEBUG
static int sock = -1;
#endif
FILE *nxlinkout;

int nxLinkInitialize(void)
{
    #ifdef NXLINK_DEBUG
    int ret = -1;
    struct sockaddr_in srv_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (!sock) {
        return ret;
    }

    bzero(&srv_addr, sizeof srv_addr);
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr = __nxlink_host;
    srv_addr.sin_port = htons(NXLINK_CLIENT_PORT);

    ret = connect(sock, (struct sockaddr *) &srv_addr, sizeof(srv_addr));
    if (ret != 0) {
        close(sock);
        return -1;
    }

    fflush(nxlinkout);
    nxlinkout = fdopen(sock, "w");
    setvbuf(nxlinkout, NULL, _IONBF, 0);
    #endif
    return 0;
}

void nxLinkExit(void)
{
    #ifdef NXLINK_DEBUG
    fclose(nxlinkout);
    #endif
}

void printBytes(FILE* out, u8 *bytes, size_t size, bool includeHeader)
{
    if (out == NULL)
        return;

    int count = 0;

    if (includeHeader)
    {
        fprintf(out, "\n\n00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
        fprintf(out, "-----------------------------------------------\n");
    }

    for (int i = 0; i < size; i++)
    {
        fprintf(out, "%02x ", bytes[i]);
        count++;
        if ((count % 16) == 0)
            fprintf(out, "\n");
    }

    fprintf(out, "\n");
}