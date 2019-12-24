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

#include <atomic>
#include <switch/types.h>
#include <memory>

#include "nx/ncm.hpp"
#include "nx/nca_writer.h"

namespace tin::data
{
    static const size_t BUFFER_SEGMENT_DATA_SIZE = 0x800000; // Approximately 8MB
    extern int NUM_BUFFER_SEGMENTS;

    struct BufferSegment
    {
        std::atomic_bool isFinalized = false;
        u64 writeOffset = 0;
        u8 data[BUFFER_SEGMENT_DATA_SIZE] = {0};
    };

    // Receives data in a circular buffer split into 8MB segments
    class BufferedPlaceholderWriter
    {
        private:
            size_t m_totalDataSize = 0;
            size_t m_sizeBuffered = 0;
            size_t m_sizeWrittenToPlaceholder = 0;

            // The current segment to which further data will be appended
            u64 m_currentFreeSegment = 0;
            BufferSegment* m_currentFreeSegmentPtr = NULL;
            // The current segment that will be written to the placeholder
            u64 m_currentSegmentToWrite = 0;
            BufferSegment* m_currentSegmentToWritePtr = NULL;

            std::unique_ptr<BufferSegment[]> m_bufferSegments;

            std::shared_ptr<nx::ncm::ContentStorage> m_contentStorage;
            NcmContentId m_ncaId;
			NcaWriter m_writer;

        public:
            BufferedPlaceholderWriter(std::shared_ptr<nx::ncm::ContentStorage>& contentStorage, NcmContentId ncaId, size_t totalDataSize);

            void AppendData(void* source, size_t length);
            bool CanAppendData(size_t length);

            void WriteSegmentToPlaceholder();
            bool CanWriteSegmentToPlaceholder();

            // Determine the number of segments required to fit data of this size
            u32 CalcNumSegmentsRequired(size_t size);

            // Check if there are enough free segments to fit data of this size
            bool IsSizeAvailable(size_t size);

            bool IsBufferDataComplete();
            bool IsPlaceholderComplete();

            size_t GetTotalDataSize();
            size_t GetSizeBuffered();
            size_t GetSizeWrittenToPlaceholder();

            void DebugPrintBuffers();
    };
}