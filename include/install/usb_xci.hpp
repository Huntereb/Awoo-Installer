#pragma once

#include <string>
#include "install/xci.hpp"

namespace tin::install::xci
{
    class USBXCI : public XCI
    {
        private:
            std::string m_xciName;

        public:
            USBXCI(std::string xciName);

            virtual void StreamToPlaceholder(std::shared_ptr<nx::ncm::ContentStorage>& contentStorage, NcmContentId placeholderId) override;
            virtual void BufferData(void* buf, off_t offset, size_t size) override;
    };
}