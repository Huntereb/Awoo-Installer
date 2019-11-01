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

#include <switch/types.h>
#include "data/byte_buffer.hpp"

namespace tin::data
{
    class ByteStream
    {
        protected:
            u64 m_offset = 0;

        public:
            virtual void ReadBytes(void* dest, size_t length) = 0;
    };

    // NOTE: This isn't generally useful, it's mainly for things like libpng
    // which rely  on streams
    class BufferedByteStream : public ByteStream
    {
        private:
            ByteBuffer m_byteBuffer;

        public:
            BufferedByteStream(ByteBuffer buffer);

            void ReadBytes(void* dest, size_t length) override;
    };
}