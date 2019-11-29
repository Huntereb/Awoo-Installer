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