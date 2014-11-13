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

// #ifdef EXT_REGS

#include "main.h"
#include "cpu.h"
#include "types.h"

uint32_t PROGRAM_COUNTER, * CP0,*FPCR,*RegRDRAM,*RegSP,*RegDPC,*RegMI,*RegVI,*RegAI,*RegPI,
	*RegRI,*RegSI, HalfLine, RegModValue, ViFieldNumber, LLBit, LLAddr;
void * FPRDoubleLocation[32], * FPRFloatLocation[32];
MIPS_DWORD *GPR, *FPR, HI, LO;
int32_t fpuControl;
N64_REGISTERS * Registers = 0;

void SetupRegisters(N64_REGISTERS * n64_Registers) {
	PROGRAM_COUNTER = n64_Registers->PROGRAM_COUNTER;
	HI.DW    = n64_Registers->HI.DW;
	LO.DW    = n64_Registers->LO.DW;
	CP0      = n64_Registers->CP0;
	GPR      = n64_Registers->GPR;
	FPR      = n64_Registers->FPR;
	FPCR     = n64_Registers->FPCR;
	RegRDRAM = n64_Registers->RDRAM;
	RegSP    = n64_Registers->SP;
	RegDPC   = n64_Registers->DPC;
	RegMI    = n64_Registers->MI;
	RegVI    = n64_Registers->VI;
	RegAI    = n64_Registers->AI;
	RegPI    = n64_Registers->PI;
	RegRI    = n64_Registers->RI;
	RegSI    = n64_Registers->SI;
	PIF_Ram  = n64_Registers->PIF_Ram;
}

void ChangeMiIntrMask (void) {
	if ( ( RegModValue & MI_INTR_MASK_CLR_SP ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_SP; }
	if ( ( RegModValue & MI_INTR_MASK_SET_SP ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_SP; }
	if ( ( RegModValue & MI_INTR_MASK_CLR_SI ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_SI; }
	if ( ( RegModValue & MI_INTR_MASK_SET_SI ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_SI; }
	if ( ( RegModValue & MI_INTR_MASK_CLR_AI ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_AI; }
	if ( ( RegModValue & MI_INTR_MASK_SET_AI ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_AI; }
	if ( ( RegModValue & MI_INTR_MASK_CLR_VI ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_VI; }
	if ( ( RegModValue & MI_INTR_MASK_SET_VI ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_VI; }
	if ( ( RegModValue & MI_INTR_MASK_CLR_PI ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_PI; }
	if ( ( RegModValue & MI_INTR_MASK_SET_PI ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_PI; }
	if ( ( RegModValue & MI_INTR_MASK_CLR_DP ) != 0 ) { MI_INTR_MASK_REG &= ~MI_INTR_MASK_DP; }
	if ( ( RegModValue & MI_INTR_MASK_SET_DP ) != 0 ) { MI_INTR_MASK_REG |= MI_INTR_MASK_DP; }
}

void ChangeMiModeReg (void) {
	MI_MODE_REG &= ~0x7F;
	MI_MODE_REG |= (RegModValue & 0x7F);
	if ( ( RegModValue & MI_CLR_INIT ) != 0 ) { MI_MODE_REG &= ~MI_MODE_INIT; }
	if ( ( RegModValue & MI_SET_INIT ) != 0 ) { MI_MODE_REG |= MI_MODE_INIT; }
	if ( ( RegModValue & MI_CLR_EBUS ) != 0 ) { MI_MODE_REG &= ~MI_MODE_EBUS; }
	if ( ( RegModValue & MI_SET_EBUS ) != 0 ) { MI_MODE_REG |= MI_MODE_EBUS; }
	if ( ( RegModValue & MI_CLR_DP_INTR ) != 0 ) { MI_INTR_REG &= ~MI_INTR_DP; }
	if ( ( RegModValue & MI_CLR_RDRAM ) != 0 ) { MI_MODE_REG &= ~MI_MODE_RDRAM; }
	if ( ( RegModValue & MI_SET_RDRAM ) != 0 ) { MI_MODE_REG |= MI_MODE_RDRAM; }
}

void ChangeSpStatus (void) {
	if ( ( RegModValue & SP_CLR_HALT ) != 0) { SP_STATUS_REG &= ~SP_STATUS_HALT; }
	if ( ( RegModValue & SP_SET_HALT ) != 0) { SP_STATUS_REG |= SP_STATUS_HALT;  }
	if ( ( RegModValue & SP_CLR_BROKE ) != 0) { SP_STATUS_REG &= ~SP_STATUS_BROKE; }
	if ( ( RegModValue & SP_CLR_INTR ) != 0) {
		MI_INTR_REG &= ~MI_INTR_SP;
		CheckInterrupts();
	}

	if ( ( RegModValue & SP_CLR_SSTEP ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SSTEP; }
	if ( ( RegModValue & SP_SET_SSTEP ) != 0) { SP_STATUS_REG |= SP_STATUS_SSTEP;  }
	if ( ( RegModValue & SP_CLR_INTR_BREAK ) != 0) { SP_STATUS_REG &= ~SP_STATUS_INTR_BREAK; }
	if ( ( RegModValue & SP_SET_INTR_BREAK ) != 0) { SP_STATUS_REG |= SP_STATUS_INTR_BREAK;  }
	if ( ( RegModValue & SP_CLR_SIG0 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG0; }
	if ( ( RegModValue & SP_SET_SIG0 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG0;  }
	if ( ( RegModValue & SP_CLR_SIG1 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG1; }
	if ( ( RegModValue & SP_SET_SIG1 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG1;  }
	if ( ( RegModValue & SP_CLR_SIG2 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG2; }
	if ( ( RegModValue & SP_SET_SIG2 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG2;  }
	if ( ( RegModValue & SP_CLR_SIG3 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG3; }
	if ( ( RegModValue & SP_SET_SIG3 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG3;  }
	if ( ( RegModValue & SP_CLR_SIG4 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG4; }
	if ( ( RegModValue & SP_SET_SIG4 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG4;  }
	if ( ( RegModValue & SP_CLR_SIG5 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG5; }
	if ( ( RegModValue & SP_SET_SIG5 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG5;  }
	if ( ( RegModValue & SP_CLR_SIG6 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG6; }
	if ( ( RegModValue & SP_SET_SIG6 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG6;  }
	if ( ( RegModValue & SP_CLR_SIG7 ) != 0) { SP_STATUS_REG &= ~SP_STATUS_SIG7; }
	if ( ( RegModValue & SP_SET_SIG7 ) != 0) { SP_STATUS_REG |= SP_STATUS_SIG7;  }

	RunRsp();

}

void UpdateCurrentHalfLine (void) {
	if (Timers->Timer < 0) {
		HalfLine = 0;
		return;
	}
	HalfLine = (Timers->Timer / 1500);
	HalfLine &= ~1;
	HalfLine += ViFieldNumber;

}

void SetFpuLocations (void) {
    int count;
    
    if ((STATUS_REGISTER & STATUS_FR) == 0) {
        for (count = 0; count < 32; count ++) {
            FPRFloatLocation[count] = (void *)(&FPR[count >> 1].W[count & 1]);
            //FPRDoubleLocation[count] = FPRFloatLocation[count];
            FPRDoubleLocation[count] = (void *)(&FPR[count >> 1].DW);
        }
    } else {
        for (count = 0; count < 32; count ++) {
            FPRFloatLocation[count] = (void *)(&FPR[count].W[1]);
            //FPRFloatLocation[count] = (void *)(&FPR[count].W[1]);
            //FPRDoubleLocation[count] = FPRFloatLocation[count];
            FPRDoubleLocation[count] = (void *)(&FPR[count].DW);
        }
    }
}
