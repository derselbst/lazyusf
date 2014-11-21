#include <stdint.h>
#include "usf.h"
#include "cpu.h"
#include "memory.h"
#include "audio.h"

char *RAM_Pages[0x81];
char *ROM_Pages[0x400];
char *IMEM, *DMEM, *RDRAM;
int RamSize, RomSize;
int *TLB_Map;

int RamBytes = 0, RomBytes = 0;
unsigned long PageVRAM(unsigned long x) {
	int paddr = ((unsigned long) (TLB_Map[x >> TLB_GRAN] + x + 0x75c) );
	return (unsigned long) (RAM_Pages[paddr >> 16]+(paddr & 0xffff));
}


int InitMemory() {
	int i = 0;
	RamSize = 0x400000;
	RomSize = 0;

	for(i = 0; i < 0x81; i++)
		RAM_Pages[i] = 0;

	for(i = 0; i < 0x400; i++)
		ROM_Pages[i] = 0;

	TLB_Map = (int*) malloc(TLB_SIZE * sizeof(int*));

	memset(TLB_Map, 0, (TLB_SIZE * sizeof(int*)));
	
// 	N64MEM = mmap((uintptr_t)TLB_Map + 0x100000 * sizeof(uintptr_t) + 0x10000, 0xD000 + RdramSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, 0, 0);
	RDRAM = (uintptr_t)TLB_Map + 0x100000 * sizeof(uintptr_t) + 0x10000;
    if(RDRAM == NULL)
    {
        printf("Failed to allocate RDRAM");
        return 0;
    }


    return 1;
}


int FreeMemory() {
	int i = 0;
	RamSize = 0x400000;
	RomSize = 0;

	for(i = 0; i < 0x81; i++) {
		if(RAM_Pages[i])
			free(RAM_Pages[i]);
		RAM_Pages[i] = 0;
	}

	for(i = 0; i < 0x400; i++) {
		if(ROM_Pages[i])
			free(ROM_Pages[i]);
		ROM_Pages[i] = 0;
	}

	if(TLB_Map) {
		free(TLB_Map);
		TLB_Map = 0;
	}


    return 1;
}


int SetupTLB(int start) {
	unsigned long i = 0;

	if(start) {
		for (i = 0x80000000; i < 0xC0000000; i += TLB_GRAN2)
			TLB_Map[i >> TLB_GRAN] = (i & 0x1FFFFFFF) - i;
	}

	for(i = 0; i < 32; i++) {
		unsigned long VirtualStart = 0, PhysicalStart = 0, VirtualEnd = 0, PhysicalEnd = 0, j = 0, add = 0;
		if(state->tlb[i].EntryDefined==0)
			continue;

		VirtualStart = state->tlb[i].EntryHi.VPN2 << 13;
		PhysicalStart = state->tlb[i].EntryLo0.PFN << 12;
		VirtualEnd = VirtualStart + (state->tlb[i].PageMask.Mask << 12) + 0xFFF;

		if (state->tlb[i].EntryLo0.V)
			add = 1;

		if((VirtualStart >= 0x80000000) && (VirtualEnd < 0xC0000000) || (PhysicalStart >= 0x20000000))
			add = 0;

		if(add) {
			for(j = VirtualStart; j < VirtualEnd; j += TLB_GRAN2)
				TLB_Map[j >> TLB_GRAN] = ((j - VirtualStart) + PhysicalStart) - j;
		}

		add = 0;

		VirtualStart = VirtualEnd + 1;
		PhysicalStart = state->tlb[i].EntryLo1.PFN << 12;
		VirtualEnd = VirtualStart + (state->tlb[i].PageMask.Mask << 12) + 0xFFF;

		if (state->tlb[i].EntryLo1.V)
			add = 1;

		if((VirtualStart >= 0x80000000) && (VirtualEnd < 0xC0000000) || (PhysicalStart >= 0x20000000))
			add = 0;

		if(add) {
			for(j = VirtualStart; j < VirtualEnd; j += TLB_GRAN2)
				TLB_Map[j >> TLB_GRAN] = ((j - VirtualStart) + PhysicalStart) - j;
		}
	}

	return 1;
}

int LD_VAddr(unsigned long addr, long long *value) {

	unsigned long address = TLB_Map[addr >> TLB_GRAN] + addr + 0x75c;
	long long temp= 0;

	if (TLB_Map[addr >> TLB_GRAN] == 0)
		return 0;

	if((address - 0x75c) > state->RamSize) {
		return 0;
	} else {
		if(RAM_Pages[address >> 16] == 0) {
			value = 0;
			return 1;
		}

		*((unsigned long *)(value) + 1) = *(unsigned long *)(RAM_Pages[(address) >> 16] + ((address) & 0xffff));

		*((unsigned long *)(value)) = *(unsigned long *)(RAM_Pages[(address+4) >> 16] + ((address+4) & 0xffff));

	}
	return 1;
}


int SD_VAddr(unsigned long addr, long long value) {

	unsigned long address = TLB_Map[addr >> TLB_GRAN] + addr + 0x75c;

	if (TLB_Map[addr >> TLB_GRAN] == 0)
		return 0;

	if((address - 0x75c) > state->RamSize) {
		return 0;
	} else {
		if(RAM_Pages[address >> 16] == 0) {
			RAM_Pages[address >> 16] = malloc(0x10000);
			RamBytes += 0x10000;
		}

		*(unsigned long *)(RAM_Pages[(address) >> 16] + ((address) & 0xffff)) = *((unsigned long *)(&value) + 1);
		*(unsigned long *)(RAM_Pages[(address+4) >> 16] + ((address+4) & 0xffff)) = *((unsigned long *)(&value));

	}
	return 1;
}


int LW_VAddr(unsigned long addr, unsigned long *value) {

	unsigned long address = TLB_Map[addr >> TLB_GRAN] + addr + 0x75c;

	if(addr < 0x400000) {
		//address = addr&0x7fffff;
		//*value = *(unsigned long *)(RAM_Pages[address >> 16] + (address & 0xffff));
		//
		return 1;
	}

	if (TLB_Map[addr >> TLB_GRAN] == 0)
		return 0;

	if((address - 0x75c) > state->RamSize) {
		return LW_Register(address-0x75c, value);
	} else {
		if(RAM_Pages[address >> 16] == 0) {
				value = 0;
			return 1;
		}

		*value = *(unsigned long *)(RAM_Pages[address >> 16] + (address & 0xffff));
	}
	return 1;
}


int SW_VAddr(unsigned long addr, unsigned long value) {

	unsigned long address = TLB_Map[addr >> TLB_GRAN] + addr + 0x75c;

	if (TLB_Map[addr >> TLB_GRAN] == 0)
		return 0;

	if((address - 0x75c) > state->RamSize) {
		return SW_Register(address-0x75c, value);
	} else {
		if(RAM_Pages[address >> 16] == 0) {
			RAM_Pages[address >> 16] = malloc(0x10000);
			RamBytes += 0x10000;
		}

		*(unsigned long *)(RAM_Pages[address >> 16] + (address & 0xffff)) = value;
	}
	return 1;
}


int LH_VAddr(unsigned long addr, unsigned short *value) {

	unsigned long address = TLB_Map[addr >> TLB_GRAN] + (addr ^ 2) + 0x75c;

	if (TLB_Map[addr >> TLB_GRAN] == 0)
		return 0;

	if((address - 0x75c) > state->RamSize) {
		return 0;
	} else {
		if(RAM_Pages[address >> 16] == 0) {
			*value = 0;
			return 1;
		}

		*value = *(unsigned short *)(RAM_Pages[address >> 16] + (address & 0xffff));
	}
	return 1;
}


int SH_VAddr(unsigned long addr, unsigned short value) {

	unsigned long address = TLB_Map[addr >> TLB_GRAN] + (addr ^ 2) + 0x75c;

	if (TLB_Map[addr >> TLB_GRAN] == 0)
		return 0;

	if((address - 0x75c) > state->RamSize) {
		return 0;

	} else {
		if(RAM_Pages[address >> 16] == 0) {
			RAM_Pages[address >> 16] = malloc(0x10000);
			RamBytes += 0x10000;
		}

		*(unsigned short *)(RAM_Pages[address >> 16] + (address & 0xffff)) = value;
	}
	return 1;
}


int LB_VAddr(unsigned long addr, unsigned char *value) {

	unsigned long address = TLB_Map[addr >> TLB_GRAN] + (addr ^ 3) + 0x75c;

	if (TLB_Map[addr >> TLB_GRAN] == 0)
		return 0;

	if((address - 0x75c) > state->RamSize) {
		return 0;
	} else {
		if(RAM_Pages[address >> 16] == 0) {
			RAM_Pages[address >> 16] = malloc(0x10000);
			RamBytes += 0x10000;
		}

		*value = *(unsigned char *)(RAM_Pages[address >> 16] + (address & 0xffff));
	}
	return 1;
}

int SB_VAddr(unsigned long addr, unsigned char value) {

	unsigned long address = TLB_Map[addr >> TLB_GRAN] + (addr ^ 3) + 0x75c;

	if (TLB_Map[addr >> TLB_GRAN] == 0)
		return 0;

	if((address - 0x75c) > state->RamSize) {
		return 0;
	} else {
		if(RAM_Pages[address >> 16] == 0) {

			return 1;
		}

		*(unsigned char *)(RAM_Pages[address >> 16] + (address & 0xffff)) = value;
	}
	return 1;
}

#define ASSERT_REG(reg,max,mask)	((reg>max)?max:reg)

int LW_Register(unsigned long addr, unsigned long *value) {

	unsigned long addrcalc = 0, regnum = 0;
	//ROM area
	if ((addr >= 0x10000000) && (addr < 0x16000000)) {
		if ((addr - 0x10000000) < 0x4000000) {
			*value = *(unsigned long*)PageROM(addr - 0x10000000);
			return 1;
		} else {
			*value = addr & 0xFFFF;
			*value = (*value << 16) | *value;
			return 0;
		}
	}

	regnum = (addr&0xFFFF) >> 2;

	switch(addr & 0x0FFF0000) {
		case 0x03F00000: *value = state->RegRDRAM[ASSERT_REG(regnum,10,0x3FF)]; break;
		case 0x04040000: *value = state->RegSP[ASSERT_REG(regnum,4,0x70)]; break;
		case 0x04080000: *value = state->RegSP[ASSERT_REG(regnum,1,1)]; break;
		case 0x04100000: *value = state->RegDP[ASSERT_REG(regnum,5,0xF8)]; break;
		case 0x04300000: *value = state->RegMI[ASSERT_REG(regnum,4,0xf)]; break;
		case 0x04400000:
			*value = state->RegVI[ASSERT_REG(regnum,13,0x3FFF)];
			if(regnum==4) {
				if (Timers.Timer < 0)
					HalfLine = 0;
				else {
					HalfLine = (Timers.Timer / 1500);
					HalfLine &= ~1;
					HalfLine += ViFieldNumber;
				}
				*value = HalfLine;
			}
			break;
		case 0x4500000:
			*value = state->RegAI[ASSERT_REG(regnum,3,0xa)];
			if(regnum==1) {
				*value = AiReadLength();
				return 1;
			}
			//printf("AiReg : %08x\t%08x\n", addr , state->PC);
			break;
		case 0x4600000: *value = state->RegPI[ASSERT_REG(regnum,9,0x1FF0)]; break;
		case 0x4700000: *value = state->RegRI[ASSERT_REG(regnum,7,0xff)]; break;
		case 0x4800000: if(regnum==6) *value = state->RegSI[3]; break;
		case 0x8000000: *value = 0; break;
		default: {
			unsigned long address = TLB_Map[state->PC >> 16] + (state->PC & 0xffff);
			printf("Reading from unhandled register %08x from %08x (%08x)!\n",addr, state->PC, address);
			*value = 0;
			return 1;
			}
			break;
	}

	return 1;

}

int SW_Register(unsigned long addr, unsigned long value) {
	unsigned long addrcalc = 0, regnum = 0;
	regnum = (addr&0xFFFF) >> 2;

	switch(addr & 0xFFFF0000) {
		case 0x3F00000: state->RegRDRAM[ASSERT_REG(regnum,10,0x3FF)] = value; break;
		case 0x4040000:

			if(regnum==4) {
                if ( ( value & SP_CLR_HALT ) != 0) { SP_STATUS_REG &= ~SP_STATUS_HALT; }
				if ( ( value & SP_SET_HALT ) != 0) { SP_STATUS_REG |= SP_STATUS_HALT;  }
				if ( ( value & SP_CLR_BROKE ) != 0) { SP_STATUS_REG &= ~SP_STATUS_BROKE; }
				if ( ( value & SP_CLR_INTR ) != 0) {
					MI_INTR_REG &= ~MI_INTR_SP;
					CheckInterrupts();
				}
				if ( ( value & SP_CLR_SSTEP ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SSTEP; }
				if ( ( value & SP_SET_SSTEP ) != 0) { SP_STATUS_REG |= SP_STATUS_SSTEP;  }
				if ( ( value & SP_CLR_INTR_BREAK ) != 0) { SP_STATUS_REG &= ~SP_STATUS_INTR_BREAK; }
				if ( ( value & SP_SET_INTR_BREAK ) != 0) { SP_STATUS_REG |= SP_STATUS_INTR_BREAK;  }
				if ( ( value & SP_CLR_SIG0 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG0; }
				if ( ( value & SP_SET_SIG0 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG0;  }
				if ( ( value & SP_CLR_SIG1 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG1; }
				if ( ( value & SP_SET_SIG1 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG1;  }
				if ( ( value & SP_CLR_SIG2 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG2; }
				if ( ( value & SP_SET_SIG2 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG2;  }
				if ( ( value & SP_CLR_SIG3 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG3; }
				if ( ( value & SP_SET_SIG3 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG3;  }
				if ( ( value & SP_CLR_SIG4 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG4; }
				if ( ( value & SP_SET_SIG4 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG4;  }
				if ( ( value & SP_CLR_SIG5 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG5; }
				if ( ( value & SP_SET_SIG5 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG5;  }
				if ( ( value & SP_CLR_SIG6 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG6; }
				if ( ( value & SP_SET_SIG6 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG6;  }
				if ( ( value & SP_CLR_SIG7 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG7; }
				if ( ( value & SP_SET_SIG7 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG7;  }

				RunRsp();
				CheckInterrupts();
				return 1;
			}
			state->RegSP[ASSERT_REG(regnum,4,0x70)] = value;

			if(regnum==2) { SP_DMA_READ(); return 1; }//printf("SP_DMA_READ\n");
			if(regnum==3) { printf("SP_DMA_WRITE\n"); return 0;}
			//printf("SP Value %08x > %08x\n", value, addr);

			break;
		case 0x4080000: state->RegSP[ASSERT_REG(regnum,1,1)] = value; break;
		case 0x4100000:
			if(regnum==3)  {
				//printf("SW DPC_STATUS_REG\n");
				if ( ( value & DPC_CLR_XBUS_DMEM_DMA ) != 0) { DPC_STATUS_REG &= ~DPC_STATUS_XBUS_DMEM_DMA; }
				if ( ( value & DPC_SET_XBUS_DMEM_DMA ) != 0) { DPC_STATUS_REG |= DPC_STATUS_XBUS_DMEM_DMA;  }
				if ( ( value & DPC_CLR_FREEZE ) != 0) { DPC_STATUS_REG &= ~DPC_STATUS_FREEZE; }
				if ( ( value & DPC_SET_FREEZE ) != 0) { DPC_STATUS_REG |= DPC_STATUS_FREEZE;  }
				if ( ( value & DPC_CLR_FLUSH ) != 0) { DPC_STATUS_REG &= ~DPC_STATUS_FLUSH; }
				if ( ( value & DPC_SET_FLUSH ) != 0) { DPC_STATUS_REG |= DPC_STATUS_FLUSH;  }
				if ( ( value & DPC_CLR_FREEZE ) != 0) {
					if ( ( SP_STATUS_REG & SP_STATUS_HALT ) == 0) {
						if ( ( SP_STATUS_REG & SP_STATUS_BROKE ) == 0 ) {
							RunRsp();
						}
					}
				}
				return 1;
			}
			state->RegDP[ASSERT_REG(regnum,5,0xF8)] = value;
			break;
		case 0x4300000:
			if(regnum==0) {
				//printf("SW MI_MODE_REG\n");
				MI_MODE_REG &= ~0x7F;
				MI_MODE_REG |= (value & 0x7F);
				if ( ( value & MI_CLR_INIT ) != 0 ) { MI_MODE_REG &= ~MI_MODE_INIT; }
				if ( ( value & MI_SET_INIT ) != 0 ) { MI_MODE_REG |= MI_MODE_INIT; }
				if ( ( value & MI_CLR_EBUS ) != 0 ) { MI_MODE_REG &= ~MI_MODE_EBUS; }
				if ( ( value & MI_SET_EBUS ) != 0 ) { MI_MODE_REG |= MI_MODE_EBUS; }
				if ( ( value & MI_CLR_DP_INTR ) != 0 ) {
					MI_INTR_REG &= ~MI_INTR_DP;
					CheckInterrupts();
				}
				if ( ( value & MI_CLR_RDRAM ) != 0 ) { MI_MODE_REG &= ~MI_MODE_RDRAM; }
				if ( ( value & MI_SET_RDRAM ) != 0 ) { MI_MODE_REG |= MI_MODE_RDRAM; }
				return 1;
			}
			if(regnum==3) {


				if ( ( value & MI_INTR_MASK_CLR_SP ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_SP; }
				if ( ( value & MI_INTR_MASK_SET_SP ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_SP; }
				if ( ( value & MI_INTR_MASK_CLR_SI ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_SI; }
				if ( ( value & MI_INTR_MASK_SET_SI ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_SI; }
				if ( ( value & MI_INTR_MASK_CLR_AI ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_AI; }
				if ( ( value & MI_INTR_MASK_SET_AI ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_AI; }
				if ( ( value & MI_INTR_MASK_CLR_VI ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_VI; }
				if ( ( value & MI_INTR_MASK_SET_VI ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_VI; }
				if ( ( value & MI_INTR_MASK_CLR_PI ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_PI; }
				if ( ( value & MI_INTR_MASK_SET_PI ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_PI; }
				if ( ( value & MI_INTR_MASK_CLR_DP ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_DP; }
				if ( ( value & MI_INTR_MASK_SET_DP ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_DP; }
                return 1;
			}

			state->RegMI[ASSERT_REG(regnum,4,0xf)] = value;
			break;
		case 0x4400000:
			//if(regnum==3) printf("SW VI Interrupt\n");
			if(regnum==4) { MI_INTR_REG &= ~MI_INTR_VI; CheckInterrupts(); return 1; }
			state->RegVI[ASSERT_REG(regnum,13,0x3FFF)] = value;

			break;
		case 0x4500000:
			if(regnum==1) { AiLenChanged(value); return 1; }
			state->RegAI[ASSERT_REG(regnum,3,0xa)] = value;
			if(regnum==3) {
				//printf("Clear AI Interrupt\n");
				MI_INTR_REG &= ~MI_INTR_AI;
				AudioIntrReg &= ~MI_INTR_AI;
				CheckInterrupts();
			}
			if(regnum==4) printf("AiDacrateChanged\n");
			break;
		case 0x4600000:
			if(regnum==4) { if(value&2) state->RegMI[2]&=0xFFFFFFEF; return 1; }
			state->RegPI[ASSERT_REG(regnum,9,0x1FF)] = value;
			if(regnum==2) printf("PI_DMA_READ (%08x)\n", PROGRAM_COUNTER);
			if(regnum==3) PI_DMA_WRITE();
			break;
		case 0x4700000: state->RegRI[ASSERT_REG(regnum,7,0xff)] = value; break;
		case 0x4800000:
			state->RegSI[ASSERT_REG(regnum,7,0x53)] = value;

			if(regnum==1) {
                if(!enablecompare) {
					MI_INTR_REG |= MI_INTR_SI;
					SI_STATUS_REG |= SI_STATUS_INTERRUPT;
					CheckInterrupts();
				}
			}


			if(regnum==4) {

				if(enablecompare) {
					MI_INTR_REG |= MI_INTR_SI;
					SI_STATUS_REG |= SI_STATUS_INTERRUPT;
					CheckInterrupts();
				}
			}

			if(regnum==6) {
				MI_INTR_REG &= ~MI_INTR_SI;
				SI_STATUS_REG &= ~SI_STATUS_INTERRUPT;
				CheckInterrupts();
				return 1;
			}
			break;
		case 0x1FC00000: printf("PifRamWrite\n"); break;
		default:
			printf("writing2 to unhandled register %08x from %08x!\n",addr, state->PC);
			return 0;
			break;
	}
	return 1;
}


int LW_PAddr(unsigned long addr, unsigned long *value) {

	unsigned long address = addr + 0x75c;

	if(RAM_Pages[address >> 16] == 0) {
		return 0;
	}

	*value = *(unsigned long *)(RAM_Pages[address >> 16] + (address & 0xffff));

	return 1;

}


int LW_PAddr_Imm(unsigned long addr) {

	unsigned long address = addr + 0x75c, value = 0;

	if(RAM_Pages[address >> 16] == 0) {
		value = 0;
		return 0;
	}

	value = *(unsigned long *)(RAM_Pages[address >> 16] + (address & 0xffff));

	return value;

}

unsigned char LB_PAddr_Imm(unsigned long addr) {

	unsigned long address = (addr^3) + 0x75c;
	unsigned char value = 0;

	if(RAM_Pages[address >> 16] == 0) {
		value = 0;
		return 0;
	}

	value = *(unsigned char *)(RAM_Pages[address >> 16] + (address & 0xffff));

	return value;

}


int SW_PAddr(unsigned long addr, unsigned long value) {

	unsigned long address = addr + 0x75c;

	if(RAM_Pages[address >> 16] == 0) {
		value = 0;
		return 1;
	}

	*(unsigned long *)(RAM_Pages[address >> 16] + (address & 0xffff)) = value;

	return 1;
}


int LH_PAddr(unsigned long addr, unsigned short *value) {

	unsigned long address = addr + 0x75c;

	if(RAM_Pages[address >> 16] == 0) {
		value = 0;
		return 1;
	}

	*value = *(unsigned short *)(RAM_Pages[address >> 16] + (address & 0xffff));

	return 1;

}


int SH_PAddr(unsigned long addr, unsigned short value) {

	unsigned long address = addr + 0x75c;

	if(RAM_Pages[address >> 16] == 0) {
		value = 0;
		return 1;
	}

	*(unsigned short *)(RAM_Pages[address >> 16] + (address & 0xffff)) = value;

	return 1;
}

int LB_PAddr(unsigned long addr, unsigned char *value) {

	unsigned long address = addr + 0x75c;

	if(RAM_Pages[address >> 16] == 0) {
		value = 0;
		return 1;
	}

	*value = *(unsigned char *)(RAM_Pages[address >> 16] + (address & 0xffff));

	return 1;

}


int SB_PAddr(unsigned long addr, unsigned char value) {

	unsigned long address = addr + 0x75c;

	if(RAM_Pages[address >> 16] == 0) {
		value = 0;
		return 1;
	}

	*(unsigned char *)(RAM_Pages[address >> 16] + (address & 0xffff)) = value;

	return 1;
}

void memcpy2n64(unsigned char *dest, unsigned char *src, int len)
{
	int i = 0;
	for(i=0; i <len; i+=4) {
		if(RAM_Pages[(unsigned long)(dest+i+0x75c) >> 16] ==0)
			RAM_Pages[(unsigned long)(dest+i+0x75c) >> 16] = malloc(0x10000);
		*(unsigned long *)(RAM_Pages[(unsigned long)(dest+i+0x75c) >> 16] + ((unsigned long)(dest+i+0x75c) & 0xffff)) = *(unsigned long*)((int)src+i);
	}
}

void memcpyn642n64(char *dest, char *src, int len)
{
	int i = 0;
	int temp = 0;

	for(i=0; i <len; i+=4) {
        unsigned long dstAddr = PageVRAM((int)dest+i);
        unsigned long srcAddr = PageVRAM((int)src+i);

		if(dstAddr < 0x75c) {
			RAM_Pages[(unsigned long)(dest+i+0x75c) >> 16] = malloc(0x10000);
			dstAddr = PageVRAM((int)dest+i);
		}

		if(srcAddr < 0x75c) {
			RAM_Pages[(unsigned long)(src+i+0x75c) >> 16] = malloc(0x10000);
			srcAddr = PageVRAM((int)src+i);
		}

		temp = *(unsigned long *)srcAddr;
		*(unsigned long *)dstAddr = temp;
	}

}


void memcpyfn64(unsigned char *dest, unsigned char *src, int len)
{
	int i = 0;
	for(i=0; i <len; i+=4) {
		if(RAM_Pages[(int)(src+i+0x75c) >> 16] ==0 )
			RAM_Pages[(int)(src+i+0x75c) >> 16] = malloc(0x10000);
			*(unsigned long*)((int)dest+i) = *(unsigned long*)(RAM_Pages[(unsigned long)(src+i+0x75c) >> 16] + ((unsigned long)(src+i+0x75c) & 0xffff));
	}
}


void WriteTlb(int index) {

	state->tlb[index].PageMask.Value = PAGE_MASK_REGISTER;
	state->tlb[index].EntryHi.Value = ENTRYHI_REGISTER;
	state->tlb[index].EntryLo0.Value = ENTRYLO0_REGISTER;
	state->tlb[index].EntryLo1.Value = ENTRYLO1_REGISTER;
	state->tlb[index].EntryDefined = 1;

	SetupTLB(0);

}
