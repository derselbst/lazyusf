
#include "cpu_hle.h"
#include "audiolib.h"

#define N64WORD(x)		(*(unsigned long*)PageVRAM((x)))
#define N64HALF(x)		(*(unsigned short*)PageVRAM((x)))
#define N64BYTE(x)		(*(unsigned char*)PageVRAM((x)))


int alCopy(int paddr) {
	unsigned long source = (state->GPR[4].UW[0]);
	unsigned long dest = (state->GPR[5].UW[0]);
	unsigned long len = (state->GPR[6].UW[0]);

	if(len&3)
		printf("OMG!!!! - alCopy length & 3\n");

	memcpyn642n64(dest,source, len);

	return 1;
}



int alLink(int paddr) {
	ALLink *element = (ALLink*)PageVRAM(state->GPR[4].UW[0]);
	ALLink *after = (ALLink*)PageVRAM(state->GPR[5].UW[0]);
	ALLink *afterNext;

	element->next = after->next;
    element->prev = state->GPR[5].UW[0];

    if (after->next) {
    	afterNext = (ALLink*)PageVRAM(after->next);
        afterNext->prev = state->GPR[4].UW[0];
	}

    after->next = state->GPR[4].UW[0];
	return 1;
}


int alUnLink(int paddr) {
	ALLink *element = (ALLink*)PageVRAM(state->GPR[4].UW[0]);
	ALLink *elementNext = (ALLink*)PageVRAM(element->next);
	ALLink *elementPrev = (ALLink*)PageVRAM(element->prev);
//	_asm int 3

	if (element->next)
        elementNext->prev = element->prev;
    if (element->prev)
        elementPrev->next = element->next;
	return 1;
}

int alEvtqPostEvent(int paddr) {
	ALEventQueue *evtq;
	ALEvent *events;

	unsigned long A0 = state->GPR[4].UW[0];
	unsigned long A1 = state->GPR[5].UW[0];
	unsigned long DeltaTime = state->GPR[6].UW[0];

	unsigned long nodeNext = 0;
	unsigned long nextItem = 0;
	unsigned long node = 0;
	unsigned long nextDelta = 0;
	unsigned long item = 0;
	unsigned long postWhere = 0;
	unsigned long NEXT = 0;
	unsigned long nextItemDelta = 0;

	evtq = (ALEventQueue *)PageVRAM(A0);
	events = (ALEvent *)PageVRAM(A1);
//_asm int 3

	NEXT = evtq->freeList.next;

	if(NEXT == 0)
		return 1;

	//DisplayError("%08x", N64WORD(0x800533E4));
	//cprintf("%08x\t%08x\n", N64WORD(0x800533D4), N64WORD(0x800533D8));

	item = NEXT;
	state->GPR[4].UW[0] = NEXT;
	alUnLink(0);

	state->GPR[4].UW[0] = A1; state->GPR[5].UW[0] = NEXT + 0xC; state->GPR[6].UW[0] = 0x10;
	alCopy(0);

    postWhere = (DeltaTime==0x7FFFFFFF)?1:0;
    nodeNext = A0;
    node = nodeNext + 8;

	while(nodeNext !=0 ) {
		nodeNext = *(unsigned long*)PageVRAM(node);

		if(nodeNext != 0) {
			nextDelta = *(unsigned long*)PageVRAM(nodeNext + 8);
			nextItem = nodeNext;
			if(DeltaTime < nextDelta) {
				*(unsigned long*)PageVRAM(item + 8) = DeltaTime;
				nextItemDelta = *(unsigned long*)PageVRAM(nextItem + 8);
				*(unsigned long*)PageVRAM(nextItem + 8) = nextItemDelta - DeltaTime;

				state->GPR[4].UW[0] = item; state->GPR[5].UW[0] = node;
				alLink(0);
				return 1;
			} else {
				node = nodeNext;
				DeltaTime -= nextDelta;
				if(node == 0)
					return 1;
			}
		}

	}

	if(postWhere == 0)
		*(unsigned long*)PageVRAM(item + 8) = DeltaTime;
	else
		*(unsigned long*)PageVRAM(item + 8) = 0;


	state->GPR[4].UW[0] = item; state->GPR[5].UW[0] = node;
	alLink(0);
	return 1;
}

int alEvtqPostEvent_Alt(int paddr) {
	return 0;
}

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif

long __nextSampleTime(long driver, long *client) {

	unsigned long c = 0;
	long deltaTime = 0x7FFFFFFF;
	*client = 0;

	for(c = N64WORD(driver); c != 0; c = N64WORD(c)) {
		int samplesLeft = N64WORD(c + 0x10);
		int curSamples = N64WORD(driver + 0x20);
		if((samplesLeft - curSamples) < deltaTime) {
			*client = c;
			deltaTime = samplesLeft - curSamples;
		}
	}

	return N64WORD((*client)+0x10);
}

long _timeToSamplesNoRound(long synth, long micros)
{
    unsigned long outputRate = N64WORD(synth+0x44);
    float tmp = ((float)micros) * outputRate / 1000000.0 + 0.5;
    //DisplayError("Smaple rate is %d", outputRate);

    return (long)tmp;
}

int byteswap(char b[4] ) {
	int out = 0;
	out += b[3];
	out += b[2] << 8;
	out += b[1] << 16;
	out += b[0] << 24;
	return out;
}

int useHLE = 0;

int alAudioFrame(int paddr) {

	ALPlayer *cl;
	unsigned long alGlobals = 0;
	unsigned long driver = 0, head = 0, deltaTime = 0, c = 0, *paramSamples, *curSamples, client = 0, handler = 0, dl = 0;
	unsigned long A0 = state->GPR[4].UW[0];
	unsigned long A1 = state->GPR[5].UW[0];
	unsigned long A2 = state->GPR[6].UW[0];
	unsigned long outLen = state->GPR[7].UW[0];
	unsigned long cmdlEnd = A0;
	unsigned long lOutBuf = A2;

	alGlobals = ((*(short*)PageRAM2(paddr + 0x8)) & 0xFFFF) << 16;		//alGlobals->drvr
	alGlobals += *(short*)PageRAM2(paddr + 0xc);
	//alGlobals = 0x80750C74;
	driver = N64WORD(alGlobals);
	paramSamples = PageVRAM(driver + 0x1c);
	curSamples = PageVRAM(driver + 0x20);

	if(N64WORD(driver) == 0) {		// if(drvr->head == 0)
		N64WORD(A1) = 0;
		state->GPR[2].UW[0] = A0;
		return 1;
	}

	for(*paramSamples = __nextSampleTime(driver, &client); (*paramSamples - *curSamples) < outLen; *paramSamples = __nextSampleTime(driver, &client)) {
		int *cSamplesLeft;
		cSamplesLeft = PageVRAM(client + 0x10);
		*paramSamples &= ~0xf;

		//run handler (not-HLE'd)
		state->GPR[4].UW[0] = client;
		RunFunction(N64WORD(client+0x8));
    	*cSamplesLeft += _timeToSamplesNoRound(driver, state->GPR[2].UW[0]);
	}

	*paramSamples &= ~0xf;

	//give us some stack
	state->GPR[0x1d].UW[0] -= 0x20;
	N64WORD(state->GPR[0x1d].UW[0]+0x4) = 0; //tmp

	while (outLen > 0) {

		long maxOutSamples = 0, nOut = 0, cmdPtr = 0, output = 0, setParam = 0, handler = 0, tmp = 0x700000;

		maxOutSamples = N64WORD(driver + 0x48);
		nOut = MIN(maxOutSamples, outLen);
		cmdPtr = cmdlEnd;//+8;

		output = N64WORD(driver + 0x38);
		setParam = N64WORD(output+8);			 // alSaveParam

		state->GPR[4].DW = output;
		state->GPR[5].DW = 0x6; // AL_FILTER_SET_DRAM
		state->GPR[6].DW = lOutBuf;
		RunFunction(setParam);

		handler = N64WORD(output+4);			 // alSavePull
		state->GPR[4].DW = output;
		state->GPR[5].DW = state->GPR[0x1d].UW[0]+0x12; //&tmp
		state->GPR[6].DW = nOut;
		state->GPR[7].DW = *curSamples;
		N64WORD(state->GPR[0x1d].UW[0]+0x10) = cmdPtr;
		RunFunction(handler);

		curSamples = PageVRAM(driver + 0x20);

		cmdlEnd = state->GPR[2].UW[0];
		outLen -= nOut;
		lOutBuf += (nOut<<2);
		*curSamples += nOut;

	}

	state->GPR[0x1d].UW[0] += 0x20;

	N64WORD(A1) = (long) ((cmdlEnd - A0) >> 3);

	state->GPR[4].UW[0] = driver;

	while( (dl = N64WORD(driver+0x14)) ) {
		state->GPR[4].UW[0] = dl;
		alUnLink(0);
		state->GPR[4].UW[0] = dl;
		state->GPR[5].UW[0] = driver + 4;
		alLink(0);
	}

	state->GPR[2].UW[0] = cmdlEnd;
	return 1;
}

