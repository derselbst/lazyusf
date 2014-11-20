#ifndef _CPU_HLE_
#define _CPU_HLE_

#include "usf.h"
#include "cpu.h"
#include "ops.h"
#include "memory.h"

typedef struct {
	char *name;
	int num;
	int length;
	long bytes[80];
	int used;
	int phys;
	int (*location)(int);
} _HLE_Entry;


int DoCPUHLE(unsigned long loc);
////////////////////////////////////////////////////////////////////
// OS Thread Stuff
// found this stuff in daedalus



#endif
