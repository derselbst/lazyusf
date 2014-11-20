#include "os.h"
#include "cpu_hle.h"


#define N64WORD(x)		(*(unsigned long*)PageVRAM((x)))
#define N64HALF(x)		(*(unsigned short*)PageVRAM((x)))
#define N64BYTE(x)		(*(unsigned char*)PageVRAM((x^3)))
#define N64DWORD(addr) (((long long)N64WORD((addr))) << 32) + N64WORD((addr)+4);


int __osRestoreInt(int n)
{
	STATUS_REGISTER |= state->GPR[0x4].UW[0];
	return 1;
}

int __osDisableInt(int n)
{
	state->GPR[0x2].UW[0] = STATUS_REGISTER & 1;
	STATUS_REGISTER &= 0xFFFFFFFE;
	return 1;
}


void osEnqueueThread(unsigned long osThreadQueueAddr, unsigned long threadVAddr)
{

	OSThread *thread = (OSThread*) PageVRAM(threadVAddr);
	OSThread *oldThread = (OSThread*) PageVRAM(osThreadQueueAddr);
	OSThread *curThread = (OSThread*) PageVRAM(oldThread->next);

	while((long)curThread->priority >= (long)thread->priority) {
		oldThread = curThread;
		curThread = PageVRAM(curThread->next);
	}

	thread->next = oldThread->next;
	oldThread->next = threadVAddr;
	thread->queue = osThreadQueueAddr;
}

int __osEnqueueThread(int n) {
	osEnqueueThread(state->GPR[4].UW[0],state->GPR[5].UW[0]);
	return 1;
}

int osStartThread(int n)
{
	OSMesgQueue *osThreadQueue = NULL;
	unsigned long osThreadQueueAddr = 0;
	unsigned long oldStatus = STATUS_REGISTER & 1;
	unsigned long osActiveThreadAddr = 0;
	unsigned long osActiveThread = 0;

	OSThread *thread = (OSThread*)PageVRAM(state->GPR[4].UW[0]);

	STATUS_REGISTER &= 0xFFFFFFFE;

	osThreadQueueAddr = ((*(int*)PageRAM2(n + 0x40)) & 0xFFFF) << 16;
	osThreadQueueAddr += *(short*)PageRAM2(n + 0x50);

	osThreadQueue = (OSMesgQueue*) PageVRAM(osThreadQueueAddr);

	if(thread->state != 8 ) {
		printf("OMG, thread state is not OS_STATE_WAITING!\n");
		exit(0);
		//asm("int 3");
	}

	thread->state = OS_STATE_RUNNABLE;
	osEnqueueThread(osThreadQueueAddr,state->GPR[4].UW[0]);

	osActiveThreadAddr = ((*(int*)PageRAM2(n + 0xDC)) & 0xFFFF) << 16;
	osActiveThreadAddr += *(short*)PageRAM2(n + 0xE0);

	osActiveThread = *(int*)PageVRAM(osActiveThreadAddr);

	if(osActiveThread==0) {
		printf("OMG, active thread is NULL!\n");
		exit(0);
		//asm("int 3");
	}

	STATUS_REGISTER |= oldStatus;
//	CheckInterrupts();


	return 1;
}


int osRecvMesg(int n)
{
	unsigned long devAddr = state->GPR[7].UW[0];
	unsigned long vAddr = state->GPR[0x11].UW[0];
	unsigned long nbytes = state->GPR[0x10].UW[0];

	unsigned long oldStatus = STATUS_REGISTER & 1;

	RunFunction(n | (PROGRAM_COUNTER & 0xF0000000));

	//DisplayError("%08x\n%08x\n%08x",devAddr, vAddr, nbytes);

	return 1;
}

// doesnt even use?
int osSetIntMask(int paddr) {
#if 0
	unsigned long globalIntMask = 0;
	unsigned long mask = STATUS_REGISTER & 0xFF01;
	unsigned long interrupts = 0;
	unsigned long intAddress = 0, newMask = 0, workMask = 0;

	unsigned long baseAddress = 0;

	globalIntMask = ((*(short*)PageRAM2(paddr + 0x8)) & 0xFFFF) << 16;
	globalIntMask += *(short*)PageRAM2(paddr + 0xc);
	globalIntMask = *(int*)PageVRAM(globalIntMask);

	interrupts = (globalIntMask ^ 0xffffffff) & 0xff00;
	mask |= interrupts;
	newMask = MI_INTR_MASK_REG;

	if(!newMask)
		newMask = ((globalIntMask >> 16) ^ 0xFFFFFFFF) & 0x3F;

	mask |= (newMask << 16);

	baseAddress = ((*(short*)PageRAM2(paddr + 0x5C)) & 0xFFFF) << 16;
	baseAddress += *(short*)PageRAM2(paddr + 0x64);
	baseAddress += ((state->GPR[4].UW[0] & 0x3F0000) & globalIntMask) >> 15;;

	MI_INTR_MASK_REG = *(short*)PageVRAM(baseAddress);

	STATUS_REGISTER = ((state->GPR[4].UW[0] & 0xff01) & (globalIntMask & 0xff00)) | (STATUS_REGISTER & 0xFFFF00FF);

#endif
	return 1;
}


int osVirtualToPhysical(int paddr) {
	unsigned long address = 0;
	unsigned long vaddr = state->GPR[4].UW[0];

	address = TLB_Map[vaddr >> TLB_GRAN] + vaddr;

	if(address < 0x800000) {
		state->GPR[2].UW[0] = address;
	} else
		state->GPR[2].UW[0] = 0xFFFFFFFF;

	return 1;
}

int osAiSetNextBuffer(int paddr) {
	unsigned long var = 0, var2 = 0;
	var = ((*(short*)PageRAM2(paddr + 0x4)) & 0xFFFF) << 16;
	var += *(short*)PageRAM2(paddr + 0x8);

	var2 = N64WORD(var);
	if(AI_CONTROL_REG & 0x80000000)
		state->GPR[2].UW[0] = -1;

	AI_DRAM_ADDR_REG = state->GPR[4].UW[0];
	AI_LEN_REG = state->GPR[5].UW[0]&0x3FFF;
	AiLenChanged(AI_LEN_REG);
	state->GPR[2].UW[0] = 0;
	return 1;
}

int saveThreadContext(int paddr) {
	unsigned long OSThreadContextAddr = 0;

	OSThreadContextAddr = ((*(short*)PageRAM2(paddr)) & 0xFFFF) << 16;
	OSThreadContextAddr += *(short*)PageRAM2(paddr + 0x4);

	OSThreadContextAddr = N64WORD(OSThreadContextAddr);

	if((PageVRAM2(OSThreadContextAddr) & 0xffff) > 0xFF00) {
		printf("OMG! Too high!");
		exit(0);
	}

	//memcpy(PageVRAM(OSThreadContextAddr),




	return 0;
}

void memcpyfn642(unsigned char *dest, unsigned char *src, int len)
{
	int i = 0;
	for(i=0; i <len; i+=4) {
		int address = PageVRAM3((unsigned long)(src+i+0x75c));
		if(RAM_Pages[(unsigned long)(address) >> 16] ==0 )
			RAM_Pages[(unsigned long)(address) >> 16] = malloc(0x10000);
		*(unsigned long*)((unsigned long)dest+i) = *(unsigned long*)(RAM_Pages[(unsigned long)(address) >> 16] + ((unsigned long)(address) & 0xffff));
	}
}


int loadThreadContext(int paddr) {
	unsigned long i = 0, OSThreadContextAddr = 0, T9 = 0, osOSThread = 0, Addr2 = 0, GlobalBitMask = 0, Tmp = 0;
	unsigned long K0 = 0, K1 = 0, T0 = 0, R1 = 0, RCP = 0, intrList = 0;
	OSThread t;
	OSThreadContextAddr = ((*(short*)PageRAM2(paddr)) & 0xFFFF) << 16;
	OSThreadContextAddr += *(short*)PageRAM2(paddr + 0x8);

	Addr2 = ((*(short*)PageRAM2(paddr + 0xC)) & 0xFFFF) << 16;
	Addr2 += *(short*)PageRAM2(paddr + 0x10);

	GlobalBitMask = ((*(short*)PageRAM2(paddr + 0x20)) & 0xFFFF) << 16;
	GlobalBitMask += *(short*)PageRAM2(paddr + 0x28);
	GlobalBitMask = N64WORD(GlobalBitMask);

	intrList = ((*(short*)PageRAM2(paddr + 0x14C + 0x0)) & 0xFFFF) << 16;
	intrList += *(short*)PageRAM2(paddr + 0x150 + 0x0);

	return 0;

	if((PageVRAM2(OSThreadContextAddr) & 0xffff) > 0xFE80) {
		printf("OMG this number is too high!!!!\n");

	}

	osOSThread = N64WORD(OSThreadContextAddr);
	T9 = N64WORD(osOSThread);

	N64WORD(OSThreadContextAddr) = T9;
	N64WORD(Addr2) = osOSThread;

	N64WORD(osOSThread + 0x10) = OS_STATE_RUNNING; //T0 is globalbitmask

	K1 = N64WORD(osOSThread + 0x118); //osOSThread.context.k0

	STATUS_REGISTER = (K1 & 0xFFFF00FF) | (GlobalBitMask & 0xFF00);

	for(i = 1; i <= 0x19; i++) {
		state->GPR[i].DW = N64DWORD(osOSThread + 0x18 + (i * 8));
	}

	for(i = 0x1C; i <= 0x1F; i++) {
		state->GPR[i].DW = N64DWORD(osOSThread + 0x8 + (i * 8));
	}

	state->LO.DW = N64DWORD(osOSThread + 0x108);
	state->HI.DW = N64DWORD(osOSThread + 0x110);

	EPC_REGISTER = N64WORD(osOSThread + 0x11C);

	state->FPCR[31] = N64WORD(osOSThread + 0x12C);

	if(N64WORD(osOSThread + 0x18)) {
		for(i = 0; i <= 30; i+=2) {
			(*(unsigned long long *)FPRDoubleLocation[i]) = N64DWORD(osOSThread + 0x130 + (i * 4));
		}
	} else {
	}

	state->GPR[0x1A].UW[0] = osOSThread;

	RCP = N64WORD(osOSThread + 0x128);

	K0 = (intrList + ((RCP & (GlobalBitMask >> 16)) << 1));
	K1 = 0;

    LH_VAddr(K0, &K1);

    // cheap hack?
    //K1 = 0xAAA;

    SW_Register(0x0430000C, K1);

	NextInstruction = JUMP;

	if ((STATUS_REGISTER & STATUS_ERL) != 0) {
		JumpToLocation = ERROREPC_REGISTER;
		STATUS_REGISTER &= ~STATUS_ERL;
	} else {
		JumpToLocation = EPC_REGISTER;
		STATUS_REGISTER &= ~STATUS_EXL;
	}

	LLBit = 0;
	CheckInterrupts();

	return 0;
}

