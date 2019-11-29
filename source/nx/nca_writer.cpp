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

#include "nx/nca_writer.h"
#include <zstd.h>
#include <string.h>
#include "util/crypto.hpp"
#include "util/config.hpp"
#include "util/title_util.hpp"
#include "install/nca.hpp"

void append(std::vector<u8>& buffer, const u8* ptr, u64 sz)
{
     u64 offset = buffer.size();
     buffer.resize(offset + sz);
     memcpy(buffer.data() + offset, ptr, sz);
}

NcaBodyWriter::NcaBodyWriter(const NcmContentId& ncaId, u64 offset, std::shared_ptr<nx::ncm::ContentStorage>& contentStorage) : m_contentStorage(contentStorage), m_ncaId(ncaId), m_offset(offset)
{
}

NcaBodyWriter::~NcaBodyWriter()
{
}

u64 NcaBodyWriter::write(const  u8* ptr, u64 sz)
{
     if(isOpen())
     {
          m_contentStorage->WritePlaceholder(*(NcmPlaceHolderId*)&m_ncaId, m_offset, (void*)ptr, sz);
          m_offset += sz;
          return sz;
     }

     return 0;
}

bool NcaBodyWriter::isOpen() const
{
     return m_contentStorage != NULL;
}


class NczHeader
{
public:
     static const u64 MAGIC = 0x4E544345535A434E;

     class Section
     {
     public:
          u64 offset;
          u64 size;
          u8 cryptoType;
          u8 padding1[7];
          u64 padding2;
          u8 cryptoKey[0x10];
          u8 cryptoCounter[0x10];
     } PACKED;

     class SectionContext : public Section
     {
     public:
          SectionContext(const Section& s) : Section(s), crypto(s.cryptoKey, Crypto::AesCtr(Crypto::swapEndian(((u64*)&s.cryptoCounter)[0])))
          {
          }

          virtual ~SectionContext()
          {
          }

          void decrypt(void* p, u64 sz, u64 offset)
          {
               if (this->cryptoType != 3)
               {
                    return;
               }

               crypto.seek(offset);
               crypto.decrypt(p, p, sz);
          }

          void encrypt(void* p, u64 sz, u64 offset)
          {
               if (this->cryptoType != 3)
               {
                    return;
               }

               crypto.seek(offset);
               crypto.encrypt(p, p, sz);
          }

          Crypto::Aes128Ctr crypto;
     };

     const bool isValid()
     {
          return m_magic == MAGIC && m_sectionCount < 0xFFFF;
     }

     const u64 size() const
     {
          return sizeof(m_magic) + sizeof(m_sectionCount) + sizeof(Section) * m_sectionCount;
     }

     const Section& section(u64 i) const
     {
          return m_sections[i];
     }

     const u64 sectionCount() const
     {
          return m_sectionCount;
     }

protected:
     u64 m_magic;
     u64 m_sectionCount;
     Section m_sections[1];
} PACKED;

class NczBodyWriter : public NcaBodyWriter
{
public:
     NczBodyWriter(const NcmContentId& ncaId, u64 offset, std::shared_ptr<nx::ncm::ContentStorage>& contentStorage) : NcaBodyWriter(ncaId, offset, contentStorage)
     {
          buffIn = malloc(buffInSize);
          buffOut = malloc(buffOutSize);

          dctx = ZSTD_createDCtx();
     }

     virtual ~NczBodyWriter()
     {
          close();

          for (auto& i : sections)
          {
               if (i)
               {
                    delete i;
                    i = NULL;
               }
          }

          if (dctx)
          {
               ZSTD_freeDCtx(dctx);
               dctx = NULL;
          }
     }

     bool close()
     {
          if (this->m_buffer.size())
          {
               processChunk(m_buffer.data(), m_buffer.size());
          }

          flush();

          return true;
     }

     bool flush()
     {
          if(!isOpen())
          {
               return false;
          }

          if (m_deflateBuffer.size())
          {
               m_contentStorage->WritePlaceholder(*(NcmPlaceHolderId*)&m_ncaId, m_offset, m_deflateBuffer.data(), m_deflateBuffer.size());
               m_offset += m_deflateBuffer.size();
               m_deflateBuffer.resize(0);
          }
          return true;
     }

     NczHeader::SectionContext& section(u64 offset)
     {
          for (u64 i = 0; i < sections.size(); i++)
          {
               if (offset >= sections[i]->offset && offset < sections[i]->offset + sections[i]->size)
               {
                    return *sections[i];
               }
          }
          return *sections[0];
     }

     bool encrypt(const void* ptr, u64 sz, u64 offset)
     {
          const u8* start = (u8*)ptr;
          const u8* end = start + sz;

          while (start < end)
          {
               auto& s = section(offset);

               u64 sectionEnd = s.offset + s.size;

               u64 chunk = offset + sz > sectionEnd ? sectionEnd - offset : sz;

               s.encrypt((void*)start, chunk, offset);

               offset += chunk;
               start += chunk;
               sz -= chunk;
          }

          return true;
     }

     u64 processChunk(const  u8* ptr, u64 sz)
     {
          ZSTD_inBuffer input = { ptr, sz, 0 };
          m_deflateBuffer.resize(sz);
          m_deflateBuffer.resize(0);

          while (input.pos < input.size)
          {
               ZSTD_outBuffer output = { buffOut, buffOutSize, 0 };
               size_t const ret = ZSTD_decompressStream(dctx, &output, &input);

               if (ZSTD_isError(ret))
               {
                    printf("%s\n", ZSTD_getErrorName(ret));
                    return false;
               }

               append(m_deflateBuffer, (const u8*)buffOut, output.pos);

               if (m_deflateBuffer.size() >= 0x1000000) // 16 MB
               {
                    encrypt(m_deflateBuffer.data(), m_deflateBuffer.size(), m_offset);

                    flush();
               }

          }

          if (m_deflateBuffer.size())
          {
               encrypt(m_deflateBuffer.data(), m_deflateBuffer.size(), m_offset);

               flush();
          }
          return 1;
     }

     u64 write(const  u8* ptr, u64 sz) override
     {
          if (!m_sectionsInitialized)
          {
               if (!m_buffer.size())
               {
                    append(m_buffer, ptr, sizeof(u64)*2);
                    ptr += sizeof(u64) * 2;
                    sz -= sizeof(u64) * 2;
               }

               auto header = (NczHeader*)m_buffer.data();

               if (m_buffer.size() + sz > header->size())
               {
                    u64 remainder = header->size() - m_buffer.size();
                    append(m_buffer, ptr, remainder);
                    ptr += remainder;
                    sz -= remainder;
               }
               else
               {
                    append(m_buffer, ptr, sz);
                    ptr += sz;
                    sz = 0;
               }

               header = (NczHeader*)m_buffer.data();

               if (m_buffer.size() == header->size())
               {
                    for (u64 i = 0; i < header->sectionCount(); i++)
                    {
                         sections.push_back(new NczHeader::SectionContext(header->section(i)));
                    }

                    m_sectionsInitialized = true;
                    m_buffer.resize(0);
               }
          }

          while (sz)
          {
               if (m_buffer.size() + sz >= 0x1000000)
               {
                    u64 chunk = 0x1000000 - m_buffer.size();
                    append(m_buffer, ptr, chunk);

                    processChunk(m_buffer.data(), m_buffer.size());
                    m_buffer.resize(0);

                    sz -= chunk;
                    ptr += chunk;
               }
               else
               {
                    append(m_buffer, ptr, sz);
                    sz = 0;
               }

          }

          return sz;
     }

     size_t const buffInSize = ZSTD_DStreamInSize();
     size_t const buffOutSize = ZSTD_DStreamOutSize();

     void* buffIn = NULL;
     void* buffOut = NULL;

     ZSTD_DCtx* dctx = NULL;

     std::vector<u8> m_buffer;
     std::vector<u8> m_deflateBuffer;

     bool m_sectionsInitialized = false;

     std::vector<NczHeader::SectionContext*> sections;
};

NcaWriter::NcaWriter(const NcmContentId& ncaId, std::shared_ptr<nx::ncm::ContentStorage>& contentStorage) : m_ncaId(ncaId), m_contentStorage(contentStorage), m_writer(NULL)
{
}

NcaWriter::~NcaWriter()
{
     close();
}

bool NcaWriter::close()
{
     if (m_writer)
     {
          m_writer = NULL;
     }
     else if(m_buffer.size())
     {
          if(isOpen())
          {
               m_contentStorage->CreatePlaceholder(m_ncaId, *(NcmPlaceHolderId*)&m_ncaId, m_buffer.size());
               m_contentStorage->WritePlaceholder(*(NcmPlaceHolderId*)&m_ncaId, 0, m_buffer.data(), m_buffer.size());
          }

          m_buffer.resize(0);
     }
     m_contentStorage = NULL;
     return true;
}

bool NcaWriter::isOpen() const
{
     return (bool)m_contentStorage;
}

u64 NcaWriter::write(const  u8* ptr, u64 sz)
{
     if (m_buffer.size() < NCA_HEADER_SIZE)
     {
          if (m_buffer.size() + sz > NCA_HEADER_SIZE)
          {
               u64 remainder = NCA_HEADER_SIZE - m_buffer.size();
               append(m_buffer, ptr, remainder);

               ptr += remainder;
               sz -= remainder;
          }
          else
          {
               append(m_buffer, ptr, sz);
               ptr += sz;
               sz = 0;
          }

          if (m_buffer.size() == NCA_HEADER_SIZE)
          {
               tin::install::NcaHeader header;
               memcpy(&header, m_buffer.data(), sizeof(header));
               Crypto::AesXtr decryptor(Crypto::Keys().headerKey, false);
               Crypto::AesXtr encryptor(Crypto::Keys().headerKey, true);
               decryptor.decrypt(&header, &header, sizeof(header), 0, 0x200);

               if (header.magic == MAGIC_NCA3)
               {
                    if(isOpen())
                    {
                         m_contentStorage->CreatePlaceholder(m_ncaId, *(NcmPlaceHolderId*)&m_ncaId, header.nca_size);
                    }
               }
               else
               {
                    throw "Invalid NCA magic";
               }

               if (header.distribution == 1)
               {
                    header.distribution = 0;
               }
               encryptor.encrypt(m_buffer.data(), &header, sizeof(header), 0, 0x200);

               if(isOpen())
               {
                    m_contentStorage->WritePlaceholder(*(NcmPlaceHolderId*)&m_ncaId, 0, m_buffer.data(), m_buffer.size());
               }
          }
     }

     if (sz)
     {
          if (!m_writer)
          {
               if (sz >= sizeof(NczHeader::MAGIC))
               {
                    if (*(u64*)ptr == NczHeader::MAGIC)
                    {
                         m_writer = std::shared_ptr<NcaBodyWriter>(new NczBodyWriter(m_ncaId, m_buffer.size(), m_contentStorage));
                    }
                    else
                    {
                         m_writer = std::shared_ptr<NcaBodyWriter>(new NcaBodyWriter(m_ncaId, m_buffer.size(), m_contentStorage));
                    }
               }
               else
               {
                    throw std::runtime_error("not enough data to read ncz header");
               }
          }

          if(m_writer)
          {
               m_writer->write(ptr, sz);
          }
          else
          {
               throw std::runtime_error("null writer");
          }
     }

     return sz;
}

