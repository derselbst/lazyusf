#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "usf.h"
#include "cpu.h"
#include "memory.h"

//extern char *

#define	TLB_GRAN	12
#define	TLB_GRAN2	(2 << (TLB_GRAN-1))
#define TLB_SIZE	(0x100000000ULL >> TLB_GRAN)

#define PageRAM(x)	((unsigned long) (RAM_Pages[(x) >> 16]+((x) & 0xffff)))
#define PageRAM2(x)	((unsigned long) (RAM_Pages[(x+0x75c) >> 16]+((x+0x75c) & 0xffff)))
#define PageVRAM2(x) ((unsigned long) (TLB_Map[(x) >> TLB_GRAN] + (x) + 0x75c))
#define PageVRAM3(x) ((unsigned long) (TLB_Map[(x) >> TLB_GRAN] + (x)))

extern char *RAM_Pages[0x81];
extern char *ROM_Pages[0x400];
extern char *IMEM, *DMEM;
extern int *TLB_Map;
extern int RamSize, RomSize;

extern int RomBytes, RamBytes;


int InitMemory(void);
int FreeMemory(void);
int SetupTLB(int start);
void WriteTlb(int index) ;
unsigned long PageVRAM(unsigned long x);
void memcpyn642n64(char *dest, char *src, int len);

int LD_VAddr(unsigned long addr, long long *value);
int SD_VAddr(unsigned long addr, long long value);
int LW_VAddr(unsigned long addr, unsigned long *value);
int SW_VAddr(unsigned long addr, unsigned long value);
int LH_VAddr(unsigned long addr, unsigned short *value);
int SH_VAddr(unsigned long addr, unsigned short value);
int LB_VAddr(unsigned long addr, unsigned char *value);
int SB_VAddr(unsigned long addr, unsigned char value);
int SW_Register(unsigned long addr, unsigned long value);
int LW_Register(unsigned long addr, unsigned long *value);

int LW_PAddr(unsigned long addr, unsigned long *value);
int SW_PAddr(unsigned long addr, unsigned long value);
int LH_PAddr(unsigned long addr, unsigned short *value);
int SH_PAddr(unsigned long addr, unsigned short value);
int LB_PAddr(unsigned long addr, unsigned char *value);
int SB_PAddr(unsigned long addr, unsigned char value);

int LW_PAddr_Imm(unsigned long addr);
unsigned char LB_PAddr_Imm(unsigned long addr);

void memcpy2n64(unsigned char *dest, unsigned char *src, int len);
void memcpyfn64(unsigned char *dest, unsigned char* src, int len);


// these structures sequestered from project64

typedef struct {
	int EntryDefined;
	union {
		unsigned long Value;
		unsigned char A[4];

		struct {
			unsigned zero : 13;
			unsigned Mask : 12;
			unsigned zero2 : 7;
		} ;

	} PageMask;

	union {
		unsigned long Value;
		unsigned char A[4];

		struct {
			unsigned ASID : 8;
			unsigned Zero : 4;
			unsigned G : 1;
			unsigned VPN2 : 19;
		};

	} EntryHi;

	union {
		unsigned long Value;
		unsigned char A[4];

		struct {
			unsigned GLOBAL: 1;
			unsigned V : 1;
			unsigned D : 1;
			unsigned C : 3;
			unsigned PFN : 20;
			unsigned ZERO: 6;
		} ;

	} EntryLo0;

	union {
		unsigned long Value;
		unsigned char A[4];

		struct {
			unsigned GLOBAL: 1;
			unsigned V : 1;
			unsigned D : 1;
			unsigned C : 3;
			unsigned PFN : 20;
			unsigned ZERO: 6;
		} ;

	} EntryLo1;
} TLB;


#endif

