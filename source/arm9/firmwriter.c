/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "firmwriter.h"
#include "mem_map.h"
#include "arm9/partitions.h"
#include "arm9/firm.h"
#include "arm9/dev.h"
#include "util.h"
#include "arm9/hardware/crypto.h"
#include "fastboot3DS_pubkey_bin.h"


const u8 sighaxNandSigs[2][256] =
{
	{
		// Sighax NAND retail signature.
		// Credits: yellows8, plutoo, derrek, profi200
		0x6E, 0xFF, 0x20, 0x9C, 0x8F, 0x4A, 0xF6, 0x1F, 0x06, 0x24, 0x13, 0xD6, 0x02, 0xCA, 0x6B, 0x4D,
		0xA1, 0xEB, 0x5A, 0xB9, 0xB6, 0xF1, 0xA2, 0xAB, 0x22, 0x6A, 0x71, 0x1D, 0xA2, 0xCC, 0xC2, 0x7C,
		0x74, 0xDE, 0x17, 0x41, 0x14, 0x3B, 0xF6, 0x90, 0x58, 0x28, 0x4C, 0xAF, 0x44, 0x4F, 0x92, 0xA4,
		0x5A, 0xAF, 0xD5, 0xA0, 0x68, 0x04, 0x33, 0x23, 0xD4, 0x8A, 0xF1, 0xD0, 0xEC, 0x05, 0x56, 0x4E,
		0xBC, 0x79, 0xB5, 0x51, 0x34, 0xE9, 0x1A, 0x86, 0xC3, 0x78, 0x8C, 0x97, 0xBC, 0x29, 0xD5, 0xA5,
		0x8A, 0x8A, 0x45, 0x25, 0x58, 0x43, 0xB8, 0x91, 0x22, 0xC7, 0x80, 0x45, 0x42, 0xF7, 0x26, 0x77,
		0xC8, 0xDA, 0x5E, 0xB7, 0x42, 0x9B, 0xAF, 0x18, 0xF7, 0xA8, 0xB0, 0x2E, 0x8B, 0xB9, 0x40, 0xFE,
		0x99, 0x0E, 0x9D, 0xC9, 0x7E, 0xDC, 0xF4, 0x9D, 0xDB, 0x18, 0x09, 0x2C, 0x28, 0x20, 0x6E, 0x74,
		0x67, 0x53, 0xCC, 0x7C, 0x6E, 0x92, 0x36, 0x2A, 0xA8, 0xD5, 0x46, 0xB3, 0x8D, 0x9E, 0x8D, 0x43,
		0x11, 0xA6, 0xB1, 0x93, 0x0D, 0xA1, 0x48, 0x97, 0x80, 0x7E, 0x30, 0x4B, 0x5E, 0x1E, 0xC0, 0x85,
		0x6E, 0xEF, 0xD6, 0x2C, 0xEA, 0xEE, 0xF2, 0x8B, 0x08, 0xBD, 0x80, 0x39, 0x7A, 0x18, 0x15, 0x60,
		0xAE, 0x6F, 0xCE, 0x39, 0xD0, 0x9C, 0x39, 0xDC, 0x3D, 0xED, 0x8C, 0x87, 0x0A, 0xB6, 0xAB, 0xCE,
		0x28, 0x94, 0x94, 0x0C, 0x0E, 0x9C, 0x41, 0x74, 0xF0, 0x13, 0x1A, 0x0D, 0xA0, 0x74, 0x7C, 0x4A,
		0x7A, 0x42, 0xC9, 0xEC, 0x34, 0x87, 0xF1, 0x09, 0xE2, 0x52, 0xB7, 0xA9, 0xB8, 0x65, 0xAE, 0x47,
		0x78, 0x95, 0xE8, 0xD6, 0xA4, 0x2A, 0x07, 0x17, 0xC4, 0x0B, 0xCC, 0xC7, 0xA7, 0x35, 0xF3, 0x3B,
		0x1E, 0x37, 0x66, 0xAB, 0x0E, 0x4B, 0x5D, 0x68, 0x1B, 0xAB, 0x41, 0x07, 0x34, 0xAB, 0x62, 0xB0
	},
	{
		// Sighax NAND dev signature. Shamelessly stolen from GodMode9.
		// Credits: SciresM, Myria, Normmatt, TuxSH
		0x88, 0x69, 0x7C, 0xDC, 0xA9, 0xD1, 0xEA, 0x31, 0x82, 0x56, 0xFC, 0xD9, 0xCE, 0xD4, 0x29, 0x64,
		0xC1, 0xE9, 0x8A, 0xBC, 0x64, 0x86, 0xB2, 0xF1, 0x28, 0xEC, 0x02, 0xE7, 0x1C, 0x5A, 0xE3, 0x5D,
		0x63, 0xD3, 0xBF, 0x12, 0x46, 0x13, 0x40, 0x81, 0xAF, 0x68, 0x75, 0x47, 0x87, 0xFC, 0xB9, 0x22,
		0x57, 0x1D, 0x7F, 0x61, 0xA3, 0x0D, 0xE4, 0xFC, 0xFA, 0x82, 0x93, 0xA9, 0xDA, 0x51, 0x23, 0x96,
		0xF1, 0x31, 0x9A, 0x36, 0x49, 0x68, 0x46, 0x4C, 0xA9, 0x80, 0x6E, 0x0A, 0x52, 0x56, 0x74, 0x86,
		0x75, 0x4C, 0xDD, 0xD4, 0xC3, 0xA6, 0x2B, 0xDC, 0xE2, 0x55, 0xE0, 0xDE, 0xEC, 0x23, 0x01, 0x29,
		0xC1, 0xBA, 0xE1, 0xAE, 0x95, 0xD7, 0x86, 0x86, 0x56, 0x37, 0xC1, 0xE6, 0x5F, 0xAE, 0x83, 0xED,
		0xF8, 0xE7, 0xB0, 0x7D, 0x17, 0xC0, 0xAA, 0xDA, 0x8F, 0x05, 0x5B, 0x64, 0x0D, 0x45, 0xAB, 0x0B,
		0xAC, 0x76, 0xFF, 0x7B, 0x34, 0x39, 0xF5, 0xA4, 0xBF, 0xE8, 0xF7, 0xE0, 0xE1, 0x03, 0xBC, 0xE9,
		0x95, 0xFA, 0xD9, 0x13, 0xFB, 0x72, 0x9D, 0x3D, 0x03, 0x0B, 0x26, 0x44, 0xEC, 0x48, 0x39, 0x64,
		0x24, 0xE0, 0x56, 0x3A, 0x1B, 0x3E, 0x6A, 0x1F, 0x68, 0x0B, 0x39, 0xFC, 0x14, 0x61, 0x88, 0x6F,
		0xA7, 0xA6, 0x0B, 0x6B, 0x56, 0xC5, 0xA8, 0x46, 0x55, 0x4A, 0xE6, 0x48, 0xFC, 0x46, 0xE3, 0x0E,
		0x24, 0x67, 0x8F, 0xAF, 0x1D, 0xC3, 0xCE, 0xB1, 0x0C, 0x2A, 0x95, 0x0F, 0x4F, 0xFA, 0x20, 0x83,
		0x23, 0x4E, 0xD8, 0xDC, 0xC3, 0x58, 0x7A, 0x6D, 0x75, 0x1A, 0x7E, 0x9A, 0xFA, 0x06, 0x15, 0x69,
		0x55, 0x08, 0x4F, 0xF2, 0x72, 0x5B, 0x69, 0x8E, 0xB1, 0x74, 0x54, 0xD9, 0xB0, 0x2B, 0x6B, 0x76,
		0xBE, 0x47, 0xAB, 0xBE, 0x20, 0x62, 0x94, 0x36, 0x69, 0x87, 0xA4, 0xCA, 0xB4, 0x2C, 0xBD, 0x0B
	}
};



s32 writeFirmPartition(const char *const part, bool replaceSig)
{
	if(memcmp(part, "firm", 4) != 0) return -1;
	if(!dev_decnand->is_active()) return -2;

	size_t partInd, sector;
	if(!partitionGetIndex(part, &partInd)) return -3;
	if(!partitionGetSectorOffset(partInd, &sector)) return -4;

	size_t firmSize;
	if(!firm_size(&firmSize)) return -5;

	u8 *firmBuf = (u8*)FIRM_LOAD_ADDR;
	if(replaceSig)
		memcpy(firmBuf + 0x100, sighaxNandSigs[CFG_UNITINFO != 0], 0x100);

	u8 *const cmpBuf = (u8*)malloc(FIRMWRITER_BLK_SIZE);
	if(!cmpBuf) return -6;

	while(firmSize)
	{
		const u32 writeSize = min(firmSize, FIRMWRITER_BLK_SIZE);

		if(!dev_decnand->write_sector(sector, writeSize>>9, firmBuf) ||
		   !dev_decnand->read_sector(sector, writeSize>>9, cmpBuf))
		{
			free(cmpBuf);
			return -7;
		}
		if(memcmp(firmBuf, cmpBuf, writeSize) != 0)
		{
			free(cmpBuf);
			return -8;
		}

		sector += writeSize>>9;
		firmSize -= writeSize;
		firmBuf += writeSize;
	}

	free(cmpBuf);

	return 0;
}

s32 loadVerifyUpdate(const char *const path, u32 *const version)
{
	if(!dev_decnand->is_active()) return -1;
	if(loadVerifyFirm(path, false, true) < 0) return UPDATE_ERR_INVALID_FIRM;

	u32 *updateBuffer = (u32*)FIRM_LOAD_ADDR;
#ifdef NDEBUG
	// Verify signature
	if(!RSA_setKey2048(3, fastboot3DS_pubkey_bin, 0x01000100) ||
	   !RSA_verify2048(updateBuffer + 0x40, updateBuffer, 0x100))
		return UPDATE_ERR_INVALID_SIG;
#endif

	// verify fastboot magic
	if(memcmp((void*)updateBuffer + 0x200, "fastboot3DS    ", 16) != 0)
		return -4;

	// Check version
	const u32 vers = *(u32*)((void*)updateBuffer + 0x210);
	if(version) *version = vers;

	size_t partInd, sector;
	if(!partitionGetIndex("firm0", &partInd)) return -5;
	if(!partitionGetSectorOffset(partInd, &sector)) return -6;

	u8 *firm0Buf = (u8*)malloc(0x200);
	if(!firm0Buf) return -7;
	if(!dev_decnand->read_sector(sector + 1, 1, firm0Buf))
	{
		free(firm0Buf);
		return -8;
	}

	// verify fastboot is installed to firm0:/
	if(memcmp(firm0Buf, "fastboot3DS    ", 16) != 0)
	{
		free(firm0Buf);
		return UPDATE_ERR_NOT_INSTALLED;
	}

	free(firm0Buf);

	if(vers < ((u32)VERS_MAJOR<<16 | VERS_MINOR)) return UPDATE_ERR_DOWNGRADE;

	return 0;
}
