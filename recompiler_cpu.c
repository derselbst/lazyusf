/*
 * Project 64 - A Nintendo 64 emulator.
 *
 * (c) Copyright 2001 zilmar (zilmar@emulation64.com) and
 * Jabo (jabo@emulation64.com).
 *
 * pj64 homepage: www.pj64.net
 *
 * Permission to use, copy, modify and distribute Project64 in both binary and
 * source form, for non-commercial purposes, is hereby granted without fee,
 * providing that this license information and copyright notice appear with
 * all copies and any derived work.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event shall the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Project64 is freeware for PERSONAL USE only. Commercial users should
 * seek permission of the copyright holders first. Commercial use includes
 * charging money for Project64 or software derived from Project64.
 *
 * The copyright holders request that bug fixes and improvements to the code
 * should be forwarded to them so if they want them.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "cpu.h"
#include "memory.h"
#include "exception.h"
#include "interpreter_cpu.h"
#include "x86.h"
#include "usf.h"
#include "tlb.h"
#include "registers.h"
#include "recompiler_cpu.h"

uint32_t GetNewTestValue( void );
void MarkCodeBlock (uint32_t PAddr);

N64_Blocks_t N64_Blocks;
uint32_t * TLBLoadAddress = 0, TargetIndex;
TARGET_INFO * TargetInfo = NULL;
BLOCK_INFO BlockInfo;

void InitilizeInitialCompilerVariable ( void)
{
	memset(&BlockInfo,0,sizeof(BlockInfo));
}

int ConstantsType (int64_t Value) {
	if (((Value >> 32) == -1) && ((Value & 0x80000000) != 0)) { return STATE_CONST_32; }
	if (((Value >> 32) == 0) && ((Value & 0x80000000) == 0)) { return STATE_CONST_32; }
	return STATE_CONST_64;
}




void FixRandomReg (void) {
//	while ((int)Registers.CP0[1] < (int)Registers.CP0[6]) {
//		Registers.CP0[1] += 32 - Registers.CP0[6];
//	}
}

uint32_t GetNewTestValue(void) {
	static uint32_t LastTest = 0;
	if (LastTest == 0xFFFFFFFF) { LastTest = 0; }
	LastTest += 1;
	return LastTest;
}

void MarkCodeBlock (uint32_t PAddr) {
	if (PAddr < RdramSize) {
		N64_Blocks.NoOfRDRamBlocks[PAddr >> 12] += 1;
	} else if (PAddr >= 0x04000000 && PAddr <= 0x04000FFC) {
		N64_Blocks.NoOfDMEMBlocks += 1;
	} else if (PAddr >= 0x04001000 && PAddr <= 0x04001FFC) {
		N64_Blocks.NoOfIMEMBlocks += 1;
	} else if (PAddr >= 0x1FC00000 && PAddr <= 0x1FC00800) {
		N64_Blocks.NoOfPifRomBlocks += 1;
	} else {
	}
}

extern uint8_t *  MemChunk;

uint32_t lastgood = 0;

