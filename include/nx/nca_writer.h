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
#include <switch.h>
#include <vector>
#include "nx/ncm.hpp"
#include <memory>

#define NCA_HEADER_SIZE 0x4000
#define MAGIC_NCA3 0x3341434E /* "NCA3" */

class NcaFsHeader
{
public:
	u8 _0x0;
	u8 _0x1;
	u8 partition_type;
	u8 fs_type;
	u8 crypt_type;
	u8 _0x5[0x3];
	u8 superblock_data[0x138];
	/*union {
		pfs0_superblock_t pfs0_superblock;
		romfs_superblock_t romfs_superblock;
		//nca0_romfs_superblock_t nca0_romfs_superblock;
		bktr_superblock_t bktr_superblock;
	};*/
	union {
		u64 section_ctr;
		struct {
			uint32_t section_ctr_low;
			uint32_t section_ctr_high;
		};
	};
	u8 _0x148[0xB8]; /* Padding. */
} PACKED;

struct NcaSectionEntry
{
	u32 media_start_offset;
	u32 media_end_offset;
	u8 _0x8[0x8]; /* Padding. */
} PACKED;

class NcaHeader
{
public:
	u8 fixed_key_sig[0x100]; /* RSA-PSS signature over header with fixed key. */
	u8 npdm_key_sig[0x100]; /* RSA-PSS signature over header with key in NPDM. */
	u32 magic;
	u8 distribution; /* System vs gamecard. */
	u8 content_type;
	u8 m_cryptoType; /* Which keyblob (field 1) */
	u8 m_kaekIndex; /* Which kaek index? */
	u64 nca_size; /* Entire archive size. */
	u64 m_titleId;
	u8 _0x218[0x4]; /* Padding. */
	union {
		uint32_t sdk_version; /* What SDK was this built with? */
		struct {
			u8 sdk_revision;
			u8 sdk_micro;
			u8 sdk_minor;
			u8 sdk_major;
		};
	};
	u8 m_cryptoType2; /* Which keyblob (field 2) */
	u8 _0x221[0xF]; /* Padding. */
	u64 m_rightsId[2]; /* Rights ID (for titlekey crypto). */
	NcaSectionEntry section_entries[4]; /* Section entry metadata. */
	u8 section_hashes[4 * 0x20]; /* SHA-256 hashes for each section header. */
	u8 m_keys[4 * 0x10]; /* Encrypted key area. */
	u8 _0x340[0xC0]; /* Padding. */
	NcaFsHeader fs_headers[4]; /* FS section headers. */
} PACKED;

class NcaBodyWriter
{
public:
	NcaBodyWriter(const NcmContentId& ncaId, u64 offset, std::shared_ptr<nx::ncm::ContentStorage>& contentStorage);
	virtual ~NcaBodyWriter();
	virtual u64 write(const  u8* ptr, u64 sz);
	
	bool isOpen() const;

protected:
	std::shared_ptr<nx::ncm::ContentStorage> m_contentStorage;
	NcmContentId m_ncaId;

	u64 m_offset;
};

class NcaWriter
{
public:
	NcaWriter(const NcmContentId& ncaId, std::shared_ptr<nx::ncm::ContentStorage>& contentStorage);
	virtual ~NcaWriter();

	bool isOpen() const;
	bool close();
	u64 write(const  u8* ptr, u64 sz);

protected:
	NcmContentId m_ncaId;
	std::shared_ptr<nx::ncm::ContentStorage> m_contentStorage;
	std::vector<u8> m_buffer;
	std::shared_ptr<NcaBodyWriter> m_writer;
};
