#include "usf.h"
#include "cpu.h"
#include "memory.h"
#include "ops.h"
#include "audio.h"
#include "audio_hle.h"
#include "cpu_hle.h"
#include "rsp.h"

// interpreter cpu swiped from project 64
long JumpToLocation = 0, LLBit = 0, NextInstruction = 0, LLAddr = 0, WaitMode = 0, AudioIntrReg = 0;
long ViFieldNumber = 0, HalfLine = 0;
OPCODE Opcode;
SaveState *state;
void * FPRDoubleLocation[32], * FPRFloatLocation[32];
CPU_ACTION CPU_Action;
SYSTEM_TIMERS Timers;
unsigned long FAKE_CAUSE_REGISTER = 0;
unsigned long long cputimer=0, hletimer=0, sleeptimer=0, lasttimer=0;

#define AddressDefined(x)	(TLB_Map[x>>TLB_GRAN])

void DoSomething ( void ) {
	if (CPU_Action.CloseCPU) {
		exit(0);
	}
	if (CPU_Action.CheckInterrupts) {
		CPU_Action.CheckInterrupts = 0;
		CheckInterrupts();
	}
	if (CPU_Action.DoInterrupt) {
		CPU_Action.DoInterrupt = 0;
		DoIntrException(0);
	}

	CPU_Action.DoSomething = 0;
	//Sleep(1);

	if (CPU_Action.DoInterrupt == 1) { CPU_Action.DoSomething = 1; }
}


void InPermLoop (void) {
	// *** Changed ***/
	if (CPU_Action.DoInterrupt) { return; }

	if (( STATUS_REGISTER & STATUS_IE  ) == 0 ) { goto InterruptsDisabled; }
	if (( STATUS_REGISTER & STATUS_EXL ) != 0 ) { goto InterruptsDisabled; }
	if (( STATUS_REGISTER & STATUS_ERL ) != 0 ) { goto InterruptsDisabled; }
	if (( STATUS_REGISTER & 0xFF00) == 0) { goto InterruptsDisabled; }

	if (Timers.Timer > 0) {
		COUNT_REGISTER += Timers.Timer + 1;
		Timers.Timer = -1;
	}
	return;

InterruptsDisabled:
	//if (UpdateScreen != NULL) { UpdateScreen(); }
	printf("Error: In permement loop at %08x\nExiting\n", PROGRAM_COUNTER);
	exit(0);
}

void ChangeCompareTimer(void) {
	unsigned long NextCompare = COMPARE_REGISTER - COUNT_REGISTER;
	if ((NextCompare & 0x80000000) != 0) {  NextCompare = 0x7FFFFFFF; }
	if (NextCompare == 0) { NextCompare = 0x1; }
	ChangeTimer(CompareTimer,NextCompare);
}

void ChangeTimer(int Type, int Value) {
	if (Value == 0) {
		Timers.NextTimer[Type] = 0;
		Timers.Active[Type] = 0;
		return;
	}
	Timers.NextTimer[Type] = Value - Timers.Timer;
	Timers.Active[Type] = 1;
	CheckTimer();
}

void CheckTimer (void) {
	int count;

	for (count = 0; count < MaxTimers; count++) {
		if (!Timers.Active[count]) { continue; }
		if (!(count == CompareTimer && Timers.NextTimer[count] == 0x7FFFFFFF)) {
			Timers.NextTimer[count] += Timers.Timer;
		}
	}
	Timers.CurrentTimerType = -1;
	Timers.Timer = 0x7FFFFFFF;
	for (count = 0; count < MaxTimers; count++) {
		if (!Timers.Active[count]) { continue; }
		if (Timers.NextTimer[count] >= Timers.Timer) { continue; }
		Timers.Timer = Timers.NextTimer[count];
		Timers.CurrentTimerType = count;
	}
	if (Timers.CurrentTimerType == -1) {
		printf("Error: No active timers ???\nEmulation Stoped at %08x\n", PROGRAM_COUNTER);
		exit(0);
	}
	for (count = 0; count < MaxTimers; count++) {
		if (!Timers.Active[count]) { continue; }
		if (!(count == CompareTimer && Timers.NextTimer[count] == 0x7FFFFFFF)) {
			Timers.NextTimer[count] -= Timers.Timer;
		}
	}

	if (Timers.NextTimer[CompareTimer] == 0x7FFFFFFF) {
		unsigned long NextCompare = COMPARE_REGISTER - COUNT_REGISTER;
		if ((NextCompare & 0x80000000) == 0 && NextCompare != 0x7FFFFFFF) {
			ChangeCompareTimer();
		}
	}
}


void CheckInterrupts ( void ) {

	MI_INTR_REG &= ~MI_INTR_AI;
	MI_INTR_REG |= (AudioIntrReg & MI_INTR_AI);
	if ((MI_INTR_MASK_REG & MI_INTR_REG) != 0) {
		FAKE_CAUSE_REGISTER |= CAUSE_IP2;
	} else  {
		FAKE_CAUSE_REGISTER &= ~CAUSE_IP2;
	}

	if (( STATUS_REGISTER & STATUS_IE   ) == 0 ) { return; }
	if (( STATUS_REGISTER & STATUS_EXL  ) != 0 ) { return; }
	if (( STATUS_REGISTER & STATUS_ERL  ) != 0 ) { return; }

	if (( STATUS_REGISTER & FAKE_CAUSE_REGISTER & 0xFF00) != 0) {
		if (!CPU_Action.DoInterrupt) {
			CPU_Action.DoSomething = 1;
			CPU_Action.DoInterrupt = 1;
		}
	}
}

void DoAddressError ( int DelaySlot, unsigned long BadVaddr, int FromRead) {

    int address = TLB_Map[BadVaddr >> TLB_GRAN] + BadVaddr;
    printf("AddressError at %08x (%08x)  Vaddr=%08x %08x %08x\n", PROGRAM_COUNTER, address, BadVaddr, state->GPR[0x8].UW[0],  state->GPR[0x9].UW[0]);
    //_asm int 3
	if (FromRead) {
		CAUSE_REGISTER = EXC_RADE;
	} else {
		CAUSE_REGISTER = EXC_WADE;
	}
	BAD_VADDR_REGISTER = BadVaddr;
	if (DelaySlot) {
		CAUSE_REGISTER |= CAUSE_BD;
		EPC_REGISTER = PROGRAM_COUNTER - 4;
	} else {
		EPC_REGISTER = PROGRAM_COUNTER;
	}
	STATUS_REGISTER |= STATUS_EXL;
	PROGRAM_COUNTER = 0x80000180;
}

void DoBreakException ( int DelaySlot) {
//  printf("DoBreakException\n");
	CAUSE_REGISTER = EXC_BREAK;
	if (DelaySlot) {
		CAUSE_REGISTER |= CAUSE_BD;
		EPC_REGISTER = PROGRAM_COUNTER - 4;
	} else {
		EPC_REGISTER = PROGRAM_COUNTER;
	}
	STATUS_REGISTER |= STATUS_EXL;
	PROGRAM_COUNTER = 0x80000180;
}

void DoCopUnusableException ( int DelaySlot, int Coprocessor ) {
   // printf("DoCopUnusableException at %08x\n", PROGRAM_COUNTER);
	CAUSE_REGISTER = EXC_CPU;
	if (Coprocessor == 1) { CAUSE_REGISTER |= 0x10000000; }
	if (DelaySlot) {
		CAUSE_REGISTER |= CAUSE_BD;
		EPC_REGISTER = PROGRAM_COUNTER - 4;
	} else {
		EPC_REGISTER = PROGRAM_COUNTER;
	}
	STATUS_REGISTER |= STATUS_EXL;
	PROGRAM_COUNTER = 0x80000180;
}

void DoIntrException ( int DelaySlot ) {
//	printf("DoIntrException\n");
	if (( STATUS_REGISTER & STATUS_IE   ) == 0 ) { return; }
	if (( STATUS_REGISTER & STATUS_EXL  ) != 0 ) { return; }
	if (( STATUS_REGISTER & STATUS_ERL  ) != 0 ) { return; }
	CAUSE_REGISTER = FAKE_CAUSE_REGISTER;
	CAUSE_REGISTER |= EXC_INT;
	if (DelaySlot) {
		CAUSE_REGISTER |= CAUSE_BD;
		EPC_REGISTER = PROGRAM_COUNTER - 4;
	} else {
		EPC_REGISTER = PROGRAM_COUNTER;
	}
	STATUS_REGISTER |= STATUS_EXL;
	PROGRAM_COUNTER = 0x80000180;
}

void DoTLBMiss ( int DelaySlot, unsigned long BadVaddr ) {
//	printf("DoTLBMiss\n");
	CAUSE_REGISTER = EXC_RMISS;
	BAD_VADDR_REGISTER = BadVaddr;
	CONTEXT_REGISTER &= 0xFF80000F;
	CONTEXT_REGISTER |= (BadVaddr >> 9) & 0x007FFFF0;
	ENTRYHI_REGISTER = (BadVaddr & 0xFFFFE000);
	if ((STATUS_REGISTER & STATUS_EXL) == 0) {
		if (DelaySlot) {
			CAUSE_REGISTER |= CAUSE_BD;
			EPC_REGISTER = PROGRAM_COUNTER - 4;
		} else {
			EPC_REGISTER = PROGRAM_COUNTER;
		}
		if (AddressDefined(BadVaddr)) {
			PROGRAM_COUNTER = 0x80000180;
		} else {
			PROGRAM_COUNTER = 0x80000000;
		}
		STATUS_REGISTER |= STATUS_EXL;
	} else {
		PROGRAM_COUNTER = 0x80000180;
	}
}

void DoSysCallException ( int DelaySlot) {
//	printf("DoSysCallException\n");
	CAUSE_REGISTER = EXC_SYSCALL;
	if (DelaySlot) {
		CAUSE_REGISTER |= CAUSE_BD;
		EPC_REGISTER = PROGRAM_COUNTER - 4;
	} else {
		EPC_REGISTER = PROGRAM_COUNTER;
	}
	STATUS_REGISTER |= STATUS_EXL;
	PROGRAM_COUNTER = 0x80000180;
}

void RefreshScreen (void ){
	static unsigned long OLD_VI_V_SYNC_REG = 0, VI_INTR_TIME = 500000;

	if (OLD_VI_V_SYNC_REG != VI_V_SYNC_REG) {
		if (VI_V_SYNC_REG == 0) {
			VI_INTR_TIME = 500000;
		} else {
			VI_INTR_TIME = (VI_V_SYNC_REG + 1) * 1500;
			if ((VI_V_SYNC_REG % 1) != 0) {
				VI_INTR_TIME -= 38;
			}
		}
	}
	ChangeTimer(ViTimer,Timers.Timer + Timers.NextTimer[ViTimer] + VI_INTR_TIME);

	if ((VI_STATUS_REG & 0x10) != 0) {
		if (ViFieldNumber == 0) {
			ViFieldNumber = 1;
		} else {
			ViFieldNumber = 0;
		}
	} else {
		ViFieldNumber = 0;
	}

}

void TimerDone (void) {
	switch (Timers.CurrentTimerType) {
	case CompareTimer:
		// this timer must be agknowledged, even with compare int disabled,
		// otherwise we get stuck in an endless loop here
		if (enablecompare)
			FAKE_CAUSE_REGISTER |= CAUSE_IP7;
		CheckInterrupts();
		ChangeCompareTimer();
		break;
	case SiTimer:
		ChangeTimer(SiTimer,0);
		MI_INTR_REG |= MI_INTR_SI;
		SI_STATUS_REG |= SI_STATUS_INTERRUPT;
		CheckInterrupts();
		break;
	case PiTimer:
		ChangeTimer(PiTimer,0);
		PI_STATUS_REG &= ~PI_STATUS_DMA_BUSY;
		MI_INTR_REG |= MI_INTR_PI;
		CheckInterrupts();
		break;
	case ViTimer: {
		RefreshScreen();
		MI_INTR_REG |= MI_INTR_VI;
		CheckInterrupts();
		WaitMode=0;
		}
		break;
	case AiTimer:
        AI_STATUS_REG=0;
		ChangeTimer(AiTimer,0);
        AudioIntrReg|=4;
		CheckInterrupts();
		break;
	}
	CheckTimer();
}

int LoadCPU(void) {
	int i = 0, count = 0;

	state = (SaveState*) PageRAM(0);

	DMEM = (char*) PageRAM(state->RamSize+0x75C);
	IMEM = (char*) DMEM + 0x1000;
	
	RSP_GPR = (REGISTER32 *)(DMEM - (32 * 8));
	RSP_ACCUM = (REGISTER *)(DMEM + 0x2000);
	RSP_Vect = (VECTOR *)((char*)RSP_ACCUM + (sizeof(REGISTER)*32));

	if(DMEM==0x75C) {
		//printf("Could not locate DMEM in segmented savestate\n\tRamSize: %08x\n\tPage: %08x	(%08x)\n\tOffset: %08x\n",
				//state->RamSize, (state->RamSize+0x75C) >> 16, RAM_Pages[(state->RamSize+0x75C) >> 16], (state->RamSize+0x75C) & 0xffff);

		RAM_Pages[(state->RamSize+0x75C) >> 16] = malloc(0x10000);
		memset(RAM_Pages[(state->RamSize+0x75C) >> 16], 0, 0x10000);

		DMEM = (char*) PageRAM(state->RamSize+0x75C);
		IMEM = (char*) DMEM + 0x1000;

	}

	SetupTLB(1);
	memset(&CPU_Action,0,sizeof(CPU_Action));

	Timers.CurrentTimerType = -1;
	Timers.Timer = 0;
	for (count = 0; count < MaxTimers; count ++) { Timers.Active[count] = 0; }

	ChangeTimer(ViTimer,state->VITimer);

	if ((STATUS_REGISTER & STATUS_FR) == 0) {
		for (i = 0; i < 32; i ++) {
			FPRFloatLocation[i] = (void *)(&state->FPR[i >> 1].W[i & 1]);
			FPRDoubleLocation[i] = (void *)(&state->FPR[i >> 1].DW);
		}
	} else {
		for (i = 0; i < 32; i ++) {
			FPRFloatLocation[i] = (void *)(&state->FPR[i].W[0]); // from 1.4
			FPRDoubleLocation[i] = (void *)(&state->FPR[i].DW);
		}
	}

	BuildInterpreter();

	ChangeCompareTimer();
	CheckInterrupts();

	Timers.CurrentTimerType = -1;
	Timers.Timer = 0;
	for (count = 0; count < MaxTimers; count ++) { Timers.Active[count] = 0; }
	ChangeTimer(ViTimer,5000);
	ChangeCompareTimer();
	ViFieldNumber = 0;

	{
		const float VSyncTiming = 789000.0f;
		double BytesPerSecond = 48681812.0 / (AI_DACRATE_REG + 1) * 4;
		double CountsPerSecond = (double)(((double)VSyncTiming) * (double)60.0);
		double CountsPerByte = (double)CountsPerSecond / (double)BytesPerSecond;
		unsigned long IntScheduled = (unsigned long)((double)AI_LEN_REG * CountsPerByte);

		if (AI_LEN_REG)
			ChangeTimer(AiTimer,IntScheduled);
	}

	AI_STATUS_REG = 0;
	AudioIntrReg = 0;

	WaitMode = 0;

	CPUHLE_Scan();

	printf("Starting emulation at %08x...\n", PROGRAM_COUNTER);

	return 1;

}

#define N64WORD(x)		(*(unsigned long*)PageVRAM((x)))
#define N64HALF(x)		(*(unsigned short*)PageVRAM((x)))
#define N64BYTE(x)		(*(unsigned char*)PageVRAM((x)))

void RunFunction(unsigned long address) {
	unsigned long oldPC = state->PC, oldRA = state->GPR[31].UW[0], la = NextInstruction;
    unsigned long callStack = 0;

    NextInstruction = NORMAL;
    PROGRAM_COUNTER = address;

    while( (state->PC != oldRA) || callStack) {

       	if(state->PC == address)
    		callStack++;

    	ExecuteInterpreterOpCode();

    	if(state->PC == oldRA)
    		callStack--;
    }

    state->PC = oldPC;
    state->GPR[31].UW[0] = oldRA;
    NextInstruction = la;

}


void ExecuteInterpreterOpCode () {

	if (WaitMode) Timers.Timer = -1;

	if (!LW_VAddr(PROGRAM_COUNTER, &Opcode.Hex)) {
		DoTLBMiss(NextInstruction == JUMP,PROGRAM_COUNTER);
		NextInstruction = NORMAL;
		return;
	}

	COUNT_REGISTER += CountPerOp;
	Timers.Timer -= CountPerOp;

	RANDOM_REGISTER -= 1;
	if ((int)RANDOM_REGISTER < (int)WIRED_REGISTER) {
		RANDOM_REGISTER = 31;
	}

	((void ( *)()) R4300i_Opcode[ Opcode.op ])();

	if (state->GPR[0].DW != 0)
		state->GPR[0].DW = 0;

	switch (NextInstruction) {
	case NORMAL:
		PROGRAM_COUNTER += 4;
		break;
	case DELAY_SLOT:
		NextInstruction = JUMP;
		PROGRAM_COUNTER += 4;
		break;
	case JUMP:

		if(!DoCPUHLE(JumpToLocation)) {
			PROGRAM_COUNTER  = JumpToLocation;
			NextInstruction = NORMAL;
		} else {
			PROGRAM_COUNTER = state->GPR[31].DW;
			NextInstruction = NORMAL;
		}

		if ((int)Timers.Timer < 0) {  TimerDone(); }
		if (CPU_Action.DoSomething) { DoSomething(); }
		break;
	}

}


#define N64DWORD(x)		(*(unsigned long long*)PageVRAM((x)))
#define N64WORD(x)		(*(unsigned long*)PageVRAM((x)))
#define N64HALF(x)		(*(unsigned short*)PageVRAM((x)))
#define N64BYTE(x)		(*(unsigned char*)PageVRAM((x)))


void StartCpu() {
	int logging;
	NextInstruction = NORMAL;

	while(!stopping) {
	    	if(PROGRAM_COUNTER==0x70003EAC)
	    		printf("A0 = %08x\tA1 = %08x\n",state->GPR[0x1a].UW[0],state->GPR[0x1b].UW[0]);
		ExecuteInterpreterOpCode();
	}
}

const unsigned char ZEROMON=0;

const unsigned char * PageROM(unsigned long addr) {
	return (ROM_Pages[addr >> 16]) ? ROM_Pages[addr >> 16] + (addr & 0xffff) : &ZEROMON;
}

void RunRsp (void) {
	if ( ( SP_STATUS_REG & SP_STATUS_HALT ) == 0) {
		if ( ( SP_STATUS_REG & SP_STATUS_BROKE ) == 0 ) {

			unsigned long Task = *( unsigned long *)PageRAM(state->RamSize+0x75C +0xFC0);

			switch (Task) {
			case 1:	{
					MI_INTR_REG |= 0x20;

					SP_STATUS_REG |= (0x0203 );
					if ((SP_STATUS_REG & SP_STATUS_INTR_BREAK) != 0 )
						MI_INTR_REG |= 1;

					CheckInterrupts();

					DPC_STATUS_REG &= ~0x0002;
					//DlistCount += 1;
				}
				break;
			case 2: {

					//do audio HLE
					if(/*!IsSeeking()*/ 1) {
						OSTask_t *task = (OSTask_t*)(DMEM + 0xFC0);
						if (audio_ucode(task)!=0)
							printf("Can't run HLE for audio.\nAudio HLE support is required for no-rsp\n");
					}

					SP_STATUS_REG |= (0x0203 );
					if ((SP_STATUS_REG & SP_STATUS_INTR_BREAK) != 0 ) {
						MI_INTR_REG |= 1;
						CheckInterrupts();
					}

				}
				break;
			default:

				break;
			}

			real_run_rsp(100);
		}
	}
}


void PI_DMA_WRITE(void) {
	PI_STATUS_REG |= PI_STATUS_DMA_BUSY;
	if ( PI_DRAM_ADDR_REG + PI_WR_LEN_REG + 1 > state->RamSize) {
		//DisplayError("PI_DMA_WRITE not in Memory");
		PI_STATUS_REG &= ~PI_STATUS_DMA_BUSY;
		MI_INTR_REG |= MI_INTR_PI;
		CheckInterrupts();
		return;
	}

	if ((PI_CART_ADDR_REG >= 0x10000000) && (PI_CART_ADDR_REG <= 0x1FBFFFFF)) {
		int i = 0;
		PI_CART_ADDR_REG -= 0x10000000;
		for (i = 0; i < PI_WR_LEN_REG + 1; i++) {
			unsigned long address = ((PI_DRAM_ADDR_REG + i) ^ 3) + 0x75c;

			if(!RAM_Pages[address >> 16])
				RAM_Pages[address >> 16] = malloc(0x100000);

			*(unsigned char*) PageRAM(address) = *PageROM((PI_CART_ADDR_REG + i) ^ 3);
		}

	PI_CART_ADDR_REG += 0x10000000;
		MI_INTR_REG |= MI_INTR_PI;
		CheckInterrupts();
		CheckTimer();
	}

	PI_STATUS_REG &= ~PI_STATUS_DMA_BUSY;
	MI_INTR_REG |= MI_INTR_PI;
	CheckInterrupts();
}

void SP_DMA_READ(void) {
	int i = 0;
	SP_DRAM_ADDR_REG &= 0x1FFFFFFF;

	//printf("SP_DMA_READing : REG=%08x  length=%08x  count=%08x  skip=%08x\n", SP_RD_LEN_REG, (SP_RD_LEN_REG&0xfff), (SP_RD_LEN_REG>>12)&0xff, (SP_RD_LEN_REG>>20)&0xfff);

	if (SP_DRAM_ADDR_REG > state->RamSize) {
		SP_DMA_BUSY_REG = 0;
		SP_STATUS_REG  &= ~SP_STATUS_DMA_BUSY;
		return;
	}

	if (SP_RD_LEN_REG + 1  + (SP_MEM_ADDR_REG & 0xFFF) > 0x1000) {
		return;
	}

	for(i=0;i<SP_RD_LEN_REG + 1; i++) {
		int iaddress = DMEM+(SP_MEM_ADDR_REG & 0x1FFF)+i;
		int oaddress = SP_DRAM_ADDR_REG + i;

		*(unsigned char*)(iaddress) = *(unsigned char*) PageRAM(oaddress+0x75c);

	}

	SP_DMA_BUSY_REG = 0;
	SP_STATUS_REG  &= ~SP_STATUS_DMA_BUSY;
	MI_INTR_REG &= ~MI_INTR_SP;
	CheckInterrupts();
	CheckTimer();
}
