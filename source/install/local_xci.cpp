#include "install/local_xci.hpp"
#include "error.hpp"
#include "debug.h"
#include "nx/nca_writer.h"
#include "nspInstall.hpp"

namespace tin::install::xci
{
    LocalXCI::LocalXCI(std::string path)
    {
        m_xciFile = fopen((path).c_str(), "rb");
        if (!m_xciFile)
            THROW_FORMAT("can't open file at %s\n", path.c_str());
    }

    LocalXCI::~LocalXCI()
    {
        fclose(m_xciFile);
    }

    void LocalXCI::StreamToPlaceholder(std::shared_ptr<nx::ncm::ContentStorage>& contentStorage, NcmContentId ncaId)
    {
        const HFS0FileEntry* fileEntry = this->GetFileEntryByNcaId(ncaId);
        std::string ncaFileName = this->GetFileEntryName(fileEntry);

        printf("Retrieving %s\n", ncaFileName.c_str());
        size_t ncaSize = fileEntry->fileSize;

        NcaWriter writer(ncaId, contentStorage);

        float progress;

        u64 fileStart = GetDataOffset() + fileEntry->dataOffset;
        u64 fileOff = 0;
        size_t readSize = 0x400000; // 4MB buff
        auto readBuffer = std::make_unique<u8[]>(readSize);

        try
        {
            inst::ui::setInstInfoText("Installing " + ncaFileName + "...");
            inst::ui::setInstBarPerc(0);
            while (fileOff < ncaSize)
            {
                progress = (float) fileOff / (float) ncaSize;

                if (fileOff % (0x400000 * 3) == 0) {
                    printf("> Progress: %lu/%lu MB (%d%s)\r", (fileOff / 1000000), (ncaSize / 1000000), (int)(progress * 100.0), "%");
                    inst::ui::setInstBarPerc((double)(progress * 100.0));
                }

                if (fileOff + readSize >= ncaSize) readSize = ncaSize - fileOff;

                this->BufferData(readBuffer.get(), fileOff + fileStart, readSize);
                writer.write(readBuffer.get(), readSize);

                fileOff += readSize;
            }
            inst::ui::setInstBarPerc(100);
        }
        catch (std::exception& e)
        {
            printf("something went wrong: %s\n", e.what());
        }

        writer.close();
    }

    void LocalXCI::BufferData(void* buf, off_t offset, size_t size)
    {
        fseeko(m_xciFile, offset, SEEK_SET);
        fread(buf, 1, size, m_xciFile);
    }
}