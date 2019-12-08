#include "install/usb_xci.hpp"


#include <switch.h>
#include <algorithm>
#include <malloc.h>
#include <threads.h>
#include "data/byte_buffer.hpp"
#include "data/buffered_placeholder_writer.hpp"
#include "util/usb_util.hpp"
#include "error.hpp"
#include "debug.h"
#include "sdInstall.hpp"
#include "util/util.hpp"
#include "util/usb_comms_awoo.h"

namespace tin::install::xci
{
    USBXCI::USBXCI(std::string xciName) :
        m_xciName(xciName)
    {

    }

    struct USBFuncArgs
    {
        std::string xciName;
        tin::data::BufferedPlaceholderWriter* bufferedPlaceholderWriter;
        u64 hfs0Offset;
        u64 ncaSize;
    };

    int USBThreadFunc(void* in)
    {
        USBFuncArgs* args = reinterpret_cast<USBFuncArgs*>(in);
        tin::util::USBCmdHeader header = tin::util::USBCmdManager::SendFileRangeCmd(args->xciName, args->hfs0Offset, args->ncaSize);

        u8* buf = (u8*)memalign(0x1000, 0x800000);
        u64 sizeRemaining = header.dataSize;
        size_t tmpSizeRead = 0;

        try
        {
            while (sizeRemaining)
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
            LOG_DEBUG("An error occurred:\n%s", e.what());
        }

        free(buf);

        return 0;
    }

    int USBPlaceholderWriteFunc(void* in)
    {
        USBFuncArgs* args = reinterpret_cast<USBFuncArgs*>(in);

        while (!args->bufferedPlaceholderWriter->IsPlaceholderComplete())
        {
            if (args->bufferedPlaceholderWriter->CanWriteSegmentToPlaceholder())
                args->bufferedPlaceholderWriter->WriteSegmentToPlaceholder();
        }

        return 0;
    }

    void USBXCI::StreamToPlaceholder(std::shared_ptr<nx::ncm::ContentStorage>& contentStorage, NcmContentId placeholderId)
    {
        const HFS0FileEntry* fileEntry = this->GetFileEntryByNcaId(placeholderId);
        std::string ncaFileName = this->GetFileEntryName(fileEntry);

        LOG_DEBUG("Retrieving %s\n", ncaFileName.c_str());
        size_t ncaSize = fileEntry->fileSize;

        tin::data::BufferedPlaceholderWriter bufferedPlaceholderWriter(contentStorage, placeholderId, ncaSize);
        USBFuncArgs args;
        args.xciName = m_xciName;
        args.bufferedPlaceholderWriter = &bufferedPlaceholderWriter;
        args.hfs0Offset = this->GetDataOffset() + fileEntry->dataOffset;
        args.ncaSize = ncaSize;
        thrd_t usbThread;
        thrd_t writeThread;

        thrd_create(&usbThread, USBThreadFunc, &args);
        thrd_create(&writeThread, USBPlaceholderWriteFunc, &args);
        
        u64 freq = armGetSystemTickFreq();
        u64 startTime = armGetSystemTick();
        size_t startSizeBuffered = 0;
        double speed = 0.0;

        inst::ui::setInstBarPerc(0);
        while (!bufferedPlaceholderWriter.IsBufferDataComplete())
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
        while (!bufferedPlaceholderWriter.IsPlaceholderComplete())
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
    }

    void USBXCI::BufferData(void* buf, off_t offset, size_t size)
    {
        LOG_DEBUG("buffering 0x%lx-0x%lx", offset, offset + size);
        tin::util::USBCmdHeader header = tin::util::USBCmdManager::SendFileRangeCmd(m_xciName, offset, size);
        u8* ourBuffer = (u8*)memalign(0x1000, header.dataSize);
        if (tin::util::USBRead(ourBuffer, header.dataSize) == 0) THROW_FORMAT("USB transfer timed out or failed");
        memcpy(buf, ourBuffer, header.dataSize);
    }
}