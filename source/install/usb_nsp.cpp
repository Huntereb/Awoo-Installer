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

#include "install/usb_nsp.hpp"


#include <switch.h>
#include <algorithm>
#include <malloc.h>
#include <threads.h>
#include "data/byte_buffer.hpp"
#include "data/buffered_placeholder_writer.hpp"
#include "util/usb_util.hpp"
#include "util/error.hpp"
#include "util/debug.h"
#include "sdInstall.hpp"
#include "util/util.hpp"
#include "util/usb_comms_awoo.h"

bool stopThreads;
std::string errorMessage;

namespace tin::install::nsp
{

    USBNSP::USBNSP(std::string nspName) :
        m_nspName(nspName)
    {

    }

    struct USBFuncArgs
    {
        std::string nspName;
        tin::data::BufferedPlaceholderWriter* bufferedPlaceholderWriter;
        u64 pfs0Offset;
        u64 ncaSize;
    };

    int USBThreadFunc(void* in)
    {
        USBFuncArgs* args = reinterpret_cast<USBFuncArgs*>(in);
        tin::util::USBCmdHeader header = tin::util::USBCmdManager::SendFileRangeCmd(args->nspName, args->pfs0Offset, args->ncaSize);

        u8* buf = (u8*)memalign(0x1000, 0x800000);
        u64 sizeRemaining = header.dataSize;
        size_t tmpSizeRead = 0;

        try
        {
            while (sizeRemaining && !stopThreads)
            {
                tmpSizeRead = awoo_usbCommsRead(buf, std::min(sizeRemaining, (u64)0x800000));
                if (tmpSizeRead == 0) THROW_FORMAT("USB transfer timed out or failed");
                sizeRemaining -= tmpSizeRead;

                while (true)
                {
                    if (args->bufferedPlaceholderWriter->CanAppendData(tmpSizeRead))
                        break;
                }

                args->bufferedPlaceholderWriter->AppendData(buf, tmpSizeRead);
            }
        }
        catch (std::exception& e)
        {
            stopThreads = true;
            errorMessage = e.what();
        }

        free(buf);

        return 0;
    }

    int USBThreadFuncNcz(void* in) // nczs corrupt with ranges over 8MB
    {
        USBFuncArgs* args = reinterpret_cast<USBFuncArgs*>(in);
        u8* buf = (u8*)memalign(0x1000, 0x800000);
        tin::util::USBCmdHeader header;
        u64 sizeRemaining = args->ncaSize;
        size_t tmpSizeRead = 0;
        u64 curOffset = 0;
        u64 curRequestLeft = 0;
        u64 reqSize = 0;
        u64 readSize = 0;

        try
        {
            while (sizeRemaining && !stopThreads)
            {
                if (!curRequestLeft) {
                    reqSize = std::min(sizeRemaining, (u64)0x800000);
                    header = tin::util::USBCmdManager::SendFileRangeCmd(args->nspName, args->pfs0Offset + curOffset, reqSize);
                    curRequestLeft = header.dataSize;
                }
                readSize = std::min(curRequestLeft, (u64)0x800000);
                tmpSizeRead = awoo_usbCommsRead(buf, readSize);
                if (tmpSizeRead == 0) THROW_FORMAT("USB transfer timed out or failed");
                curOffset += tmpSizeRead;
                sizeRemaining -= tmpSizeRead;
                curRequestLeft -= tmpSizeRead;

                while (true)
                {
                    if (args->bufferedPlaceholderWriter->CanAppendData(tmpSizeRead))
                        break;
                }

                args->bufferedPlaceholderWriter->AppendData(buf, tmpSizeRead);
            }
        }
        catch (std::exception& e)
        {
            stopThreads = true;
            errorMessage = e.what();
        }

        free(buf);

        return 0;
    }

    int USBPlaceholderWriteFunc(void* in)
    {
        USBFuncArgs* args = reinterpret_cast<USBFuncArgs*>(in);

        while (!args->bufferedPlaceholderWriter->IsPlaceholderComplete() && !stopThreads)
        {
            if (args->bufferedPlaceholderWriter->CanWriteSegmentToPlaceholder())
                args->bufferedPlaceholderWriter->WriteSegmentToPlaceholder();
        }

        return 0;
    }

    void USBNSP::StreamToPlaceholder(std::shared_ptr<nx::ncm::ContentStorage>& contentStorage, NcmContentId placeholderId)
    {
        const PFS0FileEntry* fileEntry = this->GetFileEntryByNcaId(placeholderId);
        std::string ncaFileName = this->GetFileEntryName(fileEntry);

        LOG_DEBUG("Retrieving %s\n", ncaFileName.c_str());
        size_t ncaSize = fileEntry->fileSize;

        tin::data::BufferedPlaceholderWriter bufferedPlaceholderWriter(contentStorage, placeholderId, ncaSize);
        USBFuncArgs args;
        args.nspName = m_nspName;
        args.bufferedPlaceholderWriter = &bufferedPlaceholderWriter;
        args.pfs0Offset = this->GetDataOffset() + fileEntry->dataOffset;
        args.ncaSize = ncaSize;
        thrd_t usbThread;
        thrd_t writeThread;

        stopThreads = false;
        if (m_nspName.substr(m_nspName.size() - 1, 1) == "z") thrd_create(&usbThread, USBThreadFuncNcz, &args);
        else thrd_create(&usbThread, USBThreadFunc, &args);
        thrd_create(&writeThread, USBPlaceholderWriteFunc, &args);
        
        u64 freq = armGetSystemTickFreq();
        u64 startTime = armGetSystemTick();
        size_t startSizeBuffered = 0;
        double speed = 0.0;

        inst::ui::setInstBarPerc(0);
        while (!bufferedPlaceholderWriter.IsBufferDataComplete() && !stopThreads)
        {
            u64 newTime = armGetSystemTick();

            if (newTime - startTime >= freq)
            {
                size_t newSizeBuffered = bufferedPlaceholderWriter.GetSizeBuffered();
                double mbBuffered = (newSizeBuffered / 1000000.0) - (startSizeBuffered / 1000000.0);
                double duration = ((double)(newTime - startTime) / (double)freq);
                speed =  mbBuffered / duration;

                startTime = newTime;
                startSizeBuffered = newSizeBuffered;
                int downloadProgress = (int)(((double)bufferedPlaceholderWriter.GetSizeBuffered() / (double)bufferedPlaceholderWriter.GetTotalDataSize()) * 100.0);
                #ifdef NXLINK_DEBUG
                    u64 totalSizeMB = bufferedPlaceholderWriter.GetTotalDataSize() / 1000000;
                    u64 downloadSizeMB = bufferedPlaceholderWriter.GetSizeBuffered() / 1000000;
                    LOG_DEBUG("> Download Progress: %lu/%lu MB (%i%s) (%.2f MB/s)\r", downloadSizeMB, totalSizeMB, downloadProgress, "%", speed);
                #endif

                inst::ui::setInstInfoText("Downloading " + inst::util::formatUrlString(ncaFileName) + " at " + std::to_string(speed).substr(0, std::to_string(speed).size()-4) + "MB/s");
                inst::ui::setInstBarPerc((double)downloadProgress);
            }
        }
        inst::ui::setInstBarPerc(100);

        #ifdef NXLINK_DEBUG
            u64 totalSizeMB = bufferedPlaceholderWriter.GetTotalDataSize() / 1000000;
        #endif

        inst::ui::setInstInfoText("Installing " + ncaFileName + "...");
        inst::ui::setInstBarPerc(0);
        while (!bufferedPlaceholderWriter.IsPlaceholderComplete() && !stopThreads)
        {
            int installProgress = (int)(((double)bufferedPlaceholderWriter.GetSizeWrittenToPlaceholder() / (double)bufferedPlaceholderWriter.GetTotalDataSize()) * 100.0);
            #ifdef NXLINK_DEBUG
                u64 installSizeMB = bufferedPlaceholderWriter.GetSizeWrittenToPlaceholder() / 1000000;
                LOG_DEBUG("> Install Progress: %lu/%lu MB (%i%s)\r", installSizeMB, totalSizeMB, installProgress, "%");
            #endif
            inst::ui::setInstBarPerc((double)installProgress);
        }
        inst::ui::setInstBarPerc(100);

        thrd_join(usbThread, NULL);
        thrd_join(writeThread, NULL);
        if (stopThreads) throw std::runtime_error(errorMessage.c_str());
    }

    void USBNSP::BufferData(void* buf, off_t offset, size_t size)
    {
        LOG_DEBUG("buffering 0x%lx-0x%lx\n", offset, offset + size);
        tin::util::USBCmdHeader header = tin::util::USBCmdManager::SendFileRangeCmd(m_nspName, offset, size);
        u8* tempBuffer = (u8*)memalign(0x1000, header.dataSize);
        if (tin::util::USBRead(tempBuffer, header.dataSize) == 0) THROW_FORMAT("USB transfer timed out or failed");
        memcpy(buf, tempBuffer, header.dataSize);
        free(tempBuffer);
    }
}