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
#include <cstring>
#include <vector>

namespace tin::data
{
    class ByteBuffer
    {
        private:
            std::vector<u8> m_buffer;

        public:
            ByteBuffer(size_t reserveSize=0);

            size_t GetSize();
            u8* GetData(); // TODO: Remove this, it shouldn't be needed
            void Resize(size_t size);

            void DebugPrintContents();

            template <typename T>
            T Read(u64 offset)
            {
                if (offset + sizeof(T) <= m_buffer.size())
                    return *((T*)&m_buffer.data()[offset]);

                T def;
                memset(&def, 0, sizeof(T));
                return def;
            }
            
            template <typename T>
            void Write(T data, u64 offset)
            {
                size_t requiredSize = offset + sizeof(T);

                if (requiredSize > m_buffer.size())
                    m_buffer.resize(requiredSize, 0);
                
                memcpy(m_buffer.data() + offset, &data, sizeof(T));
            }
            template <typename T>
            void Append(T data)
            {
                this->Write(data, this->GetSize());
            }
    };
}