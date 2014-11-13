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
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <sys/mman.h>


#include "main.h"
#include "cpu.h"
#include "audio.h"
#include "rsp.h"
#include "usf.h"

uintptr_t *TLB_Map = 0;
uint8_t * MemChunk = 0;
uint32_t RdramSize = 0x800000, SystemRdramSize = 0x800000, RomFileSize = 0x4000000;
uint8_t * N64MEM = 0, * RDRAM = 0, * DMEM = 0, * IMEM = 0, * ROMPages[0x400], * savestatespace = 0, * NOMEM = 0;

uint32_t WrittenToRom = 0;
uint32_t WroteToRom = 0;
uint32_t TempValue = 0;
uint32_t MemoryState = 0;

uint8_t EmptySpace = 0;

uint8_t * PageROM(uint32_t addr) {
	return (ROMPages[addr/0x10000])?ROMPages[addr/0x10000]+(addr%0x10000):&EmptySpace;
}


#define PAGE_SIZE	4096
void *malloc_exec(uint32_t bytes)
{
	void *ptr = NULL;

	ptr = mmap(0,bytes,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANON, 0, 0);

	return ptr;

}


int32_t Allocate_Memory ( void ) {
	uint32_t i = 0;
	//RdramSize = 0x800000;

	// Allocate the N64MEM and TLB_Map so that they are in each others 4GB range
	// Also put the registers there :)


	// the mmap technique works craptacular when the regions don't overlay

	MemChunk = mmap(NULL, 0x100000 * sizeof(uintptr_t) + 0x1D000 + RdramSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, 0, 0);

	TLB_Map = (uintptr_t*)MemChunk;
	if (TLB_Map == NULL) {
		return 0;
	}

	memset(TLB_Map, 0, 0x100000 * sizeof(uintptr_t) + 0x10000);

	N64MEM = mmap((uintptr_t)MemChunk + 0x100000 * sizeof(uintptr_t) + 0x10000, 0xD000 + RdramSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, 0, 0);
	if(N64MEM == NULL) {
		DisplayError("Failed to allocate N64MEM");
		return 0;
	}
	
	memset(N64MEM, 0, RdramSize);

	NOMEM = mmap((uintptr_t)N64MEM + RdramSize, 0xD000, PROT_NONE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, 0, 0);
	
	if(RdramSize == 0x400000)
	{
	//	munmap(N64MEM + 0x400000, 0x400000);
	}

	Registers = (N64_REGISTERS *)((uintptr_t)MemChunk + 0x100000 * sizeof(uintptr_t));
	//TLBLoadAddress = (uint32_t *)((uintptr_t)Registers + 0x500);
	//Timers = (SYSTEM_TIMERS*)(TLBLoadAddress + 4);
    Timers = (SYSTEM_TIMERS*)((uintptr_t)Registers + 0x500);
	WaitMode = (uint32_t *)(Timers + sizeof(SYSTEM_TIMERS));
	CPU_Action = (CPU_ACTION *)(WaitMode + 4);
	RSP_GPR = (REGISTER32 *)(CPU_Action + sizeof(CPU_ACTION));
	DMEM = (uint8_t *)(RSP_GPR + (32 * 8));
	RSP_ACCUM = (REGISTER *)(DMEM + 0x2000);
	RSP_Vect = (VECTOR *)((char*)RSP_ACCUM + (sizeof(REGISTER)*32));


	RDRAM = (uint8_t *)(N64MEM);
	IMEM  = DMEM + 0x1000;

	MemoryState = 1;

	return 1;
}

int PreAllocate_Memory(void) {
	int i = 0;
	
	// Moved the savestate allocation here :)  (for better management later)
	savestatespace = malloc(0x80275C);
	
	if(savestatespace == 0)
		return 0;
	
	memset(savestatespace, 0, 0x80275C);
	
	for (i = 0; i < 0x400; i++) {
		ROMPages[i] = 0;
	}

	return 1;
}

void Release_Memory ( void ) {
	uint32_t i;

	for (i = 0; i < 0x400; i++) {
		if (ROMPages[i]) {
			free(ROMPages[i]); ROMPages[i] = 0;
		}
	}
	//printf("Freeing memory\n");

	MemoryState = 0;

	if (MemChunk != 0) {munmap(MemChunk, 0x100000 * sizeof(uintptr_t)) + 0x1D000 + RdramSize; MemChunk=0;}
	if (N64MEM != 0) {munmap(N64MEM, RdramSize); N64MEM=0;}
	if (NOMEM != 0) {munmap(NOMEM, 0xD000); NOMEM=0;}

	if(savestatespace)
		free(savestatespace);
	savestatespace = NULL;

}


int32_t r4300i_LB_NonMemory ( uint32_t PAddr, uint32_t * Value, uint32_t SignExtend ) {
	if (PAddr >= 0x10000000 && PAddr < 0x16000000) {
		if (WrittenToRom) { return 0; }
		if ((PAddr & 2) == 0) { PAddr = (PAddr + 4) ^ 2; }
		if ((PAddr - 0x10000000) < RomFileSize) {
			if (SignExtend) {
				*Value = (char)*PageROM((PAddr - 0x10000000)^3);

			} else {
				*Value = *PageROM((PAddr - 0x10000000)^3);
			}
			return 1;
		} else {
			*Value = 0;
			return 0;
		}
	}

	switch (PAddr & 0xFFF00000) {
	default:
		* Value = 0;
		return 0;
		break;
	}
	return 1;
}

uint32_t r4300i_LB_VAddr ( uint32_t VAddr, uint8_t * Value ) {
	if (TLB_Map[VAddr >> 12] == 0) { return 0; }
	*Value = *(uint8_t *)(TLB_Map[VAddr >> 12] + (VAddr ^ 3));
	return 1;
}

uint32_t r4300i_LD_VAddr ( uint32_t VAddr, uint64_t * Value ) {
	if (TLB_Map[VAddr >> 12] == 0) { return 0; }
	*((uint32_t *)(Value) + 1) = *(uint32_t *)(TLB_Map[VAddr >> 12] + VAddr);
	*((uint32_t *)(Value)) = *(uint32_t *)(TLB_Map[VAddr >> 12] + VAddr + 4);
	return 1;
}

int32_t r4300i_LH_NonMemory ( uint32_t PAddr, uint32_t * Value, int32_t SignExtend ) {
	switch (PAddr & 0xFFF00000) {
	default:
		* Value = 0;
		return 0;
		break;
	}
	return 1;
}

uint32_t r4300i_LH_VAddr ( uint32_t VAddr, uint16_t * Value ) {
	if (TLB_Map[VAddr >> 12] == 0) { return 0; }
	*Value = *(uint16_t *)(TLB_Map[VAddr >> 12] + (VAddr ^ 2));
	return 1;
}

int32_t r4300i_LW_NonMemory ( uint32_t PAddr, uint32_t * Value ) {
	if (PAddr >= 0x10000000 && PAddr < 0x16000000) {
		if (WrittenToRom) {
			*Value = WroteToRom;
			//LogMessage("%X: Read crap from Rom %X from %X",PROGRAM_COUNTER,*Value,PAddr);
			WrittenToRom = 0;
			return 1;
		}
		if ((PAddr - 0x10000000) < RomFileSize) {
			*Value = *(uint32_t *)PageROM((PAddr - 0x10000000));
			return 1;
		} else {
			*Value = PAddr & 0xFFFF;
			*Value = (*Value << 16) | *Value;
			return 0;
		}
	}

	switch (PAddr & 0xFFF00000) {
	case 0x03F00000:
		switch (PAddr) {
		case 0x03F00000: * Value = RDRAM_CONFIG_REG; break;
		case 0x03F00004: * Value = RDRAM_DEVICE_ID_REG; break;
		case 0x03F00008: * Value = RDRAM_DELAY_REG; break;
		case 0x03F0000C: * Value = RDRAM_MODE_REG; break;
		case 0x03F00010: * Value = RDRAM_REF_INTERVAL_REG; break;
		case 0x03F00014: * Value = RDRAM_REF_ROW_REG; break;
		case 0x03F00018: * Value = RDRAM_RAS_INTERVAL_REG; break;
		case 0x03F0001C: * Value = RDRAM_MIN_INTERVAL_REG; break;
		case 0x03F00020: * Value = RDRAM_ADDR_SELECT_REG; break;
		case 0x03F00024: * Value = RDRAM_DEVICE_MANUF_REG; break;
		default:
			* Value = 0;
			return 0;
		}
		break;
	case 0x04000000:
		switch (PAddr) {
		case 0x04040010: *Value = SP_STATUS_REG; break;
		case 0x04040014: *Value = SP_DMA_FULL_REG; break;
		case 0x04040018: *Value = SP_DMA_BUSY_REG; break;
		case 0x04080000: *Value = SP_PC_REG; break;
		default:
			* Value = 0;
			return 0;
		}
		break;
	case 0x04100000:
		switch (PAddr) {
		case 0x0410000C: *Value = DPC_STATUS_REG; break;
		case 0x04100010: *Value = DPC_CLOCK_REG; break;
		case 0x04100014: *Value = DPC_BUFBUSY_REG; break;
		case 0x04100018: *Value = DPC_PIPEBUSY_REG; break;
		case 0x0410001C: *Value = DPC_TMEM_REG; break;
		default:
			* Value = 0;
			return 0;
		}
		break;
	case 0x04300000:
		switch (PAddr) {
		case 0x04300000: * Value = MI_MODE_REG; break;
		case 0x04300004: * Value = MI_VERSION_REG; break;
		case 0x04300008: * Value = MI_INTR_REG; break;
		case 0x0430000C: * Value = MI_INTR_MASK_REG; break;
		default:
			* Value = 0;
			return 0;
		}
		break;
	case 0x04400000:
		switch (PAddr) {
		case 0x04400000: *Value = VI_STATUS_REG; break;
		case 0x04400004: *Value = VI_ORIGIN_REG; break;
		case 0x04400008: *Value = VI_WIDTH_REG; break;
		case 0x0440000C: *Value = VI_INTR_REG; break;
		case 0x04400010:
			*Value = 0;
			break;
		case 0x04400014: *Value = VI_BURST_REG; break;
		case 0x04400018: *Value = VI_V_SYNC_REG; break;
		case 0x0440001C: *Value = VI_H_SYNC_REG; break;
		case 0x04400020: *Value = VI_LEAP_REG; break;
		case 0x04400024: *Value = VI_H_START_REG; break;
		case 0x04400028: *Value = VI_V_START_REG ; break;
		case 0x0440002C: *Value = VI_V_BURST_REG; break;
		case 0x04400030: *Value = VI_X_SCALE_REG; break;
		case 0x04400034: *Value = VI_Y_SCALE_REG; break;
		default:
			* Value = 0;
			return 0;
		}
		break;
	case 0x04500000:
		switch (PAddr) {
		case 0x04500004: *Value = AiReadLength(); break;
		case 0x0450000C: *Value = AI_STATUS_REG; break;
		default:
			* Value = 0;
			return 0;
		}
		break;
	case 0x04600000:
		switch (PAddr) {
		case 0x04600010: *Value = PI_STATUS_REG; break;
		case 0x04600014: *Value = PI_DOMAIN1_REG; break;
		case 0x04600018: *Value = PI_BSD_DOM1_PWD_REG; break;
		case 0x0460001C: *Value = PI_BSD_DOM1_PGS_REG; break;
		case 0x04600020: *Value = PI_BSD_DOM1_RLS_REG; break;
		case 0x04600024: *Value = PI_DOMAIN2_REG; break;
		case 0x04600028: *Value = PI_BSD_DOM2_PWD_REG; break;
		case 0x0460002C: *Value = PI_BSD_DOM2_PGS_REG; break;
		case 0x04600030: *Value = PI_BSD_DOM2_RLS_REG; break;
		default:
			* Value = 0;
			return 0;
		}
		break;
	case 0x04700000:
		switch (PAddr) {
		case 0x04700000: * Value = RI_MODE_REG; break;
		case 0x04700004: * Value = RI_CONFIG_REG; break;
		case 0x04700008: * Value = RI_CURRENT_LOAD_REG; break;
		case 0x0470000C: * Value = RI_SELECT_REG; break;
		case 0x04700010: * Value = RI_REFRESH_REG; break;
		case 0x04700014: * Value = RI_LATENCY_REG; break;
		case 0x04700018: * Value = RI_RERROR_REG; break;
		case 0x0470001C: * Value = RI_WERROR_REG; break;
		default:
			* Value = 0;
			return 0;
		}
		break;
	case 0x04800000:
		switch (PAddr) {
		case 0x04800018: *Value = SI_STATUS_REG; break;
		default:
			*Value = 0;
			return 0;
		}
		break;
	case 0x05000000:
		*Value = PAddr & 0xFFFF;
		*Value = (*Value << 16) | *Value;
		return 0;
	case 0x08000000:
		*Value = 0;
		break;
	default:
		*Value = PAddr & 0xFFFF;
		*Value = (*Value << 16) | *Value;
		return 0;
		break;
	}
	return 1;
}

void r4300i_LW_PAddr ( uint32_t PAddr, uint32_t * Value ) {
	*Value = *(uint32_t *)(N64MEM+PAddr);
}

uint32_t r4300i_LW_VAddr ( uint32_t VAddr, uint32_t * Value ) {
	uintptr_t address = (TLB_Map[VAddr >> 12] + VAddr);

	if (TLB_Map[VAddr >> 12] == 0) { return 0; }

	if((address - (uintptr_t)RDRAM) > RdramSize) {
		address = address - (uintptr_t)RDRAM;
		return r4300i_LW_NonMemory(address, Value);
	}
	*Value = *(uint32_t *)address;
	return 1;
}

int32_t r4300i_SB_NonMemory ( uint32_t PAddr, uint8_t Value ) {
	switch (PAddr & 0xFFF00000) {
	case 0x00000000:
	case 0x00100000:
	case 0x00200000:
	case 0x00300000:
	case 0x00400000:
	case 0x00500000:
	case 0x00600000:
	case 0x00700000:
		if (PAddr < RdramSize) {
			*(uint8_t *)(N64MEM+PAddr) = Value;
		}
		break;
	default:
		return 0;
		break;
	}
	return 1;
}

uint32_t r4300i_SB_VAddr ( uint32_t VAddr, uint8_t Value ) {
	if (TLB_Map[VAddr >> 12] == 0) { return 0; }
	*(uint8_t *)(TLB_Map[VAddr >> 12] + (VAddr ^ 3)) = Value;

	return 1;
}

int32_t r4300i_SH_NonMemory ( uint32_t PAddr, uint16_t Value ) {
	switch (PAddr & 0xFFF00000) {
	case 0x00000000:
	case 0x00100000:
	case 0x00200000:
	case 0x00300000:
	case 0x00400000:
	case 0x00500000:
	case 0x00600000:
	case 0x00700000:
		if (PAddr < RdramSize) {
			*(uint16_t *)(N64MEM+PAddr) = Value;
		}
		break;
	default:
		return 0;
		break;
	}
	return 1;
}

uint32_t r4300i_SD_VAddr ( uint32_t VAddr, uint64_t Value ) {
	if (TLB_Map[VAddr >> 12] == 0) { return 0; }
	*(uint32_t *)(TLB_Map[VAddr >> 12] + VAddr) = *((uint32_t *)(&Value) + 1);
	*(uint32_t *)(TLB_Map[VAddr >> 12] + VAddr + 4) = *((uint32_t *)(&Value));
	return 1;
}

uint32_t r4300i_SH_VAddr ( uint32_t VAddr, uint16_t Value ) {
	if (TLB_Map[VAddr >> 12] == 0) { return 0; }
	*(uint16_t *)(TLB_Map[VAddr >> 12] + (VAddr ^ 2)) = Value;
	return 1;
}

int32_t r4300i_SW_NonMemory ( uint32_t PAddr, uint32_t Value ) {
	if (PAddr >= 0x10000000 && PAddr < 0x16000000) {
		if ((PAddr - 0x10000000) < RomFileSize) {
			WrittenToRom = 1;
			WroteToRom = Value;
		} else {
			return 0;
		}
	}

	switch (PAddr & 0xFFF00000) {
	case 0x00000000:
	case 0x00100000:
	case 0x00200000:
	case 0x00300000:
	case 0x00400000:
	case 0x00500000:
	case 0x00600000:
	case 0x00700000:
		if (PAddr < RdramSize) {
			*(uint32_t *)(N64MEM+PAddr) = Value;
		}
		break;
	case 0x03F00000:
		switch (PAddr) {
		case 0x03F00000: RDRAM_CONFIG_REG = Value; break;
		case 0x03F00004: RDRAM_DEVICE_ID_REG = Value; break;
		case 0x03F00008: RDRAM_DELAY_REG = Value; break;
		case 0x03F0000C: RDRAM_MODE_REG = Value; break;
		case 0x03F00010: RDRAM_REF_INTERVAL_REG = Value; break;
		case 0x03F00014: RDRAM_REF_ROW_REG = Value; break;
		case 0x03F00018: RDRAM_RAS_INTERVAL_REG = Value; break;
		case 0x03F0001C: RDRAM_MIN_INTERVAL_REG = Value; break;
		case 0x03F00020: RDRAM_ADDR_SELECT_REG = Value; break;
		case 0x03F00024: RDRAM_DEVICE_MANUF_REG = Value; break;
		case 0x03F04004: break;
		case 0x03F08004: break;
		case 0x03F80004: break;
		case 0x03F80008: break;
		case 0x03F8000C: break;
		case 0x03F80014: break;
		default:
			return 0;
		}
		break;
	case 0x04000000:
		if (PAddr < 0x04002000) {
			*(uint32_t *)(N64MEM+PAddr) = Value;
			return 1;
		}
		switch (PAddr) {
		case 0x04040000: SP_MEM_ADDR_REG = Value; break;
		case 0x04040004: SP_DRAM_ADDR_REG = Value; break;
		case 0x04040008:
			SP_RD_LEN_REG = Value;
			SP_DMA_READ();
			break;
		case 0x0404000C:
			SP_WR_LEN_REG = Value;
			SP_DMA_WRITE();
			break;
		case 0x04040010:
			if ( ( Value & SP_CLR_HALT ) != 0) { SP_STATUS_REG &= ~SP_STATUS_HALT; }
			if ( ( Value & SP_SET_HALT ) != 0) { SP_STATUS_REG |= SP_STATUS_HALT;  }
			if ( ( Value & SP_CLR_BROKE ) != 0) { SP_STATUS_REG &= ~SP_STATUS_BROKE; }
			if ( ( Value & SP_CLR_INTR ) != 0) {
				MI_INTR_REG &= ~MI_INTR_SP;
				CheckInterrupts();
			}
			if ( ( Value & SP_CLR_SSTEP ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SSTEP; }
			if ( ( Value & SP_SET_SSTEP ) != 0) { SP_STATUS_REG |= SP_STATUS_SSTEP;  }
			if ( ( Value & SP_CLR_INTR_BREAK ) != 0) { SP_STATUS_REG &= ~SP_STATUS_INTR_BREAK; }
			if ( ( Value & SP_SET_INTR_BREAK ) != 0) { SP_STATUS_REG |= SP_STATUS_INTR_BREAK;  }
			if ( ( Value & SP_CLR_SIG0 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG0; }
			if ( ( Value & SP_SET_SIG0 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG0;  }
			if ( ( Value & SP_CLR_SIG1 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG1; }
			if ( ( Value & SP_SET_SIG1 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG1;  }
			if ( ( Value & SP_CLR_SIG2 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG2; }
			if ( ( Value & SP_SET_SIG2 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG2;  }
			if ( ( Value & SP_CLR_SIG3 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG3; }
			if ( ( Value & SP_SET_SIG3 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG3;  }
			if ( ( Value & SP_CLR_SIG4 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG4; }
			if ( ( Value & SP_SET_SIG4 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG4;  }
			if ( ( Value & SP_CLR_SIG5 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG5; }
			if ( ( Value & SP_SET_SIG5 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG5;  }
			if ( ( Value & SP_CLR_SIG6 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG6; }
			if ( ( Value & SP_SET_SIG6 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG6;  }
			if ( ( Value & SP_CLR_SIG7 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG7; }
			if ( ( Value & SP_SET_SIG7 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG7;  }

			RunRsp();

			break;
		case 0x0404001C: SP_SEMAPHORE_REG = 0; break;
		case 0x04080000: SP_PC_REG = Value & 0xFFC; break;
		default:
			return 0;
		}
		break;
	case 0x04100000:
		switch (PAddr) {
		case 0x04100000:
			DPC_START_REG = Value;
			DPC_CURRENT_REG = Value;
			break;
		case 0x04100004:
			DPC_END_REG = Value;
			//if (ProcessRDPList) { ProcessRDPList(); }
			break;
		case 0x04100008: DPC_CURRENT_REG = Value; break;
		case 0x0410000C:
			if ( ( Value & DPC_CLR_XBUS_DMEM_DMA ) != 0) { DPC_STATUS_REG &= ~DPC_STATUS_XBUS_DMEM_DMA; }
			if ( ( Value & DPC_SET_XBUS_DMEM_DMA ) != 0) { DPC_STATUS_REG |= DPC_STATUS_XBUS_DMEM_DMA;  }
			if ( ( Value & DPC_CLR_FREEZE ) != 0) { DPC_STATUS_REG &= ~DPC_STATUS_FREEZE; }
			if ( ( Value & DPC_SET_FREEZE ) != 0) { DPC_STATUS_REG |= DPC_STATUS_FREEZE;  }
			if ( ( Value & DPC_CLR_FLUSH ) != 0) { DPC_STATUS_REG &= ~DPC_STATUS_FLUSH; }
			if ( ( Value & DPC_SET_FLUSH ) != 0) { DPC_STATUS_REG |= DPC_STATUS_FLUSH;  }
			if ( ( Value & DPC_CLR_FREEZE ) != 0)
			{
				if ( ( SP_STATUS_REG & SP_STATUS_HALT ) == 0)
				{
					if ( ( SP_STATUS_REG & SP_STATUS_BROKE ) == 0 )
					{
						RunRsp();
					}
				}
			}
			break;
		default:
			return 0;
		}
		break;
	case 0x04300000:
		switch (PAddr) {
		case 0x04300000:
			MI_MODE_REG &= ~0x7F;
			MI_MODE_REG |= (Value & 0x7F);
			if ( ( Value & MI_CLR_INIT ) != 0 ) { MI_MODE_REG &= ~MI_MODE_INIT; }
			if ( ( Value & MI_SET_INIT ) != 0 ) { MI_MODE_REG |= MI_MODE_INIT; }
			if ( ( Value & MI_CLR_EBUS ) != 0 ) { MI_MODE_REG &= ~MI_MODE_EBUS; }
			if ( ( Value & MI_SET_EBUS ) != 0 ) { MI_MODE_REG |= MI_MODE_EBUS; }
			if ( ( Value & MI_CLR_DP_INTR ) != 0 ) {
				MI_INTR_REG &= ~MI_INTR_DP;
				CheckInterrupts();
			}
			if ( ( Value & MI_CLR_RDRAM ) != 0 ) { MI_MODE_REG &= ~MI_MODE_RDRAM; }
			if ( ( Value & MI_SET_RDRAM ) != 0 ) { MI_MODE_REG |= MI_MODE_RDRAM; }
			break;
		case 0x0430000C:
			if ( ( Value & MI_INTR_MASK_CLR_SP ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_SP; }
			if ( ( Value & MI_INTR_MASK_SET_SP ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_SP; }
			if ( ( Value & MI_INTR_MASK_CLR_SI ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_SI; }
			if ( ( Value & MI_INTR_MASK_SET_SI ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_SI; }
			if ( ( Value & MI_INTR_MASK_CLR_AI ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_AI; }
			if ( ( Value & MI_INTR_MASK_SET_AI ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_AI; }
			if ( ( Value & MI_INTR_MASK_CLR_VI ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_VI; }
			if ( ( Value & MI_INTR_MASK_SET_VI ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_VI; }
			if ( ( Value & MI_INTR_MASK_CLR_PI ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_PI; }
			if ( ( Value & MI_INTR_MASK_SET_PI ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_PI; }
			if ( ( Value & MI_INTR_MASK_CLR_DP ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_DP; }
			if ( ( Value & MI_INTR_MASK_SET_DP ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_DP; }
			break;
		default:
			return 0;
		}
		break;
	case 0x04400000:
		switch (PAddr) {
		case 0x04400000:
			//if (VI_STATUS_REG != Value) {
				VI_STATUS_REG = Value;
			//	if (ViStatusChanged != NULL ) { ViStatusChanged(); }
			//}
			break;
		case 0x04400004:

			VI_ORIGIN_REG = (Value & 0xFFFFFF);
			//if (UpdateScreen != NULL ) { UpdateScreen(); }
			break;
		case 0x04400008:
			//if (VI_WIDTH_REG != Value) {
				VI_WIDTH_REG = Value;
			//	if (ViWidthChanged != NULL ) { ViWidthChanged(); }
			//}
			break;
		case 0x0440000C: VI_INTR_REG = Value; break;
		case 0x04400010:
			MI_INTR_REG &= ~MI_INTR_VI;
			CheckInterrupts();
			break;
		case 0x04400014: VI_BURST_REG = Value; break;
		case 0x04400018: VI_V_SYNC_REG = Value; break;
		case 0x0440001C: VI_H_SYNC_REG = Value; break;
		case 0x04400020: VI_LEAP_REG = Value; break;
		case 0x04400024: VI_H_START_REG = Value; break;
		case 0x04400028: VI_V_START_REG = Value; break;
		case 0x0440002C: VI_V_BURST_REG = Value; break;
		case 0x04400030: VI_X_SCALE_REG = Value; break;
		case 0x04400034: VI_Y_SCALE_REG = Value; break;
		default:
			return 0;
		}
		break;
	case 0x04500000:
		switch (PAddr) {
		case 0x04500000: AI_DRAM_ADDR_REG = Value; break;
		case 0x04500004:
			AI_LEN_REG = Value;
			if (AiLenChanged != NULL) { AiLenChanged(); }
			break;
		case 0x04500008: AI_CONTROL_REG = (Value & 0x1); break;
		case 0x0450000C:
			/* Clear Interrupt */;
			MI_INTR_REG &= ~MI_INTR_AI;
			AudioIntrReg &= ~MI_INTR_AI;
			CheckInterrupts();
			break;
		case 0x04500010:
			AI_DACRATE_REG = Value;
			//if (AiDacrateChanged != NULL) { AiDacrateChanged(SYSTEM_NTSC); }
			break;
		case 0x04500014:  AI_BITRATE_REG = Value; break;
		default:
			return 0;
		}
		break;
	case 0x04600000:
		switch (PAddr) {
		case 0x04600000: PI_DRAM_ADDR_REG = Value; break;
		case 0x04600004: PI_CART_ADDR_REG = Value; break;
		case 0x04600008:
			PI_RD_LEN_REG = Value;
			PI_DMA_READ();
			break;
		case 0x0460000C:
			PI_WR_LEN_REG = Value;
			PI_DMA_WRITE();
			break;
		case 0x04600010:
			//if ((Value & PI_SET_RESET) != 0 ) { DisplayError("reset Controller"); }
			if ((Value & PI_CLR_INTR) != 0 ) {
				MI_INTR_REG &= ~MI_INTR_PI;
				CheckInterrupts();
			}
			break;
		case 0x04600014: PI_DOMAIN1_REG = (Value & 0xFF); break;
		case 0x04600018: PI_BSD_DOM1_PWD_REG = (Value & 0xFF); break;
		case 0x0460001C: PI_BSD_DOM1_PGS_REG = (Value & 0xFF); break;
		case 0x04600020: PI_BSD_DOM1_RLS_REG = (Value & 0xFF); break;
		default:
			return 0;
		}
		break;
	case 0x04700000:
		switch (PAddr) {
		case 0x04700000: RI_MODE_REG = Value; break;
		case 0x04700004: RI_CONFIG_REG = Value; break;
		case 0x04700008: RI_CURRENT_LOAD_REG = Value; break;
		case 0x0470000C: RI_SELECT_REG = Value; break;
		case 0x04700010: RI_REFRESH_REG = Value; break;
		case 0x04700014: RI_LATENCY_REG = Value; break;
		case 0x04700018: RI_RERROR_REG = Value; break;
		case 0x0470001C: RI_WERROR_REG = Value; break;
		default:
			return 0;
		}
		break;
	case 0x04800000:
		switch (PAddr) {
		case 0x04800000: SI_DRAM_ADDR_REG = Value; break;
		case 0x04800004:
			SI_PIF_ADDR_RD64B_REG = Value;
			SI_DMA_READ ();
			break;
		case 0x04800010:
			SI_PIF_ADDR_WR64B_REG = Value;
			SI_DMA_WRITE();
			break;
		case 0x04800018:
			MI_INTR_REG &= ~MI_INTR_SI;
			SI_STATUS_REG &= ~SI_STATUS_INTERRUPT;
			CheckInterrupts();
			break;
		default:
			return 0;
		}
		break;
	case 0x08000000:
		if (PAddr != 0x08010000) { return 0; }
		//WriteToFlashCommand(Value);
		break;
	case 0x1FC00000:
		if (PAddr < 0x1FC007C0) {
			return 0;
		} else if (PAddr < 0x1FC00800) {

			if (PAddr == 0x1FC007FC) {
				PifRamWrite();
			}
			return 1;
		}
		return 0;
		break;
	default:
		return 0;
		break;
	}
	return 1;
}

uint32_t r4300i_SW_VAddr ( uint32_t VAddr, uint32_t Value ) {
	uintptr_t address = (TLB_Map[VAddr >> 12] + VAddr);

	if (TLB_Map[VAddr >> 12] == 0) { return 0; }

	if((address - (uintptr_t)RDRAM) > RdramSize) {
		address = address - (uintptr_t)RDRAM;
		return r4300i_SW_NonMemory(address, Value);
	}
	*(uint32_t *)address = Value;
	return 1;
}
