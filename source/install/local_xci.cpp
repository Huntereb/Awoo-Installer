#include "install/local_xci.hpp"
#include "error.hpp"
#include "debug.h"

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

    bool LocalXCI::CanStream() {
        return false;
    }

    void LocalXCI::StreamToPlaceholder(std::shared_ptr<nx::ncm::ContentStorage>& contentStorage, NcmContentId placeholderId)
    {
        THROW_FORMAT("not streamable\n");
    }

    void LocalXCI::BufferData(void* buf, off_t offset, size_t size)
    {
        fseeko(m_xciFile, offset, SEEK_SET);
        fread(buf, 1, size, m_xciFile);
    }
}