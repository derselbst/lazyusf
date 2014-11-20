#ifndef _USF_H_
#define _USF_H_
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "usf.h"
#include "cpu.h"
#include "memory.h"

extern char filename[512];
extern int loaded;
extern int playing, stopping;

extern int enablecompare, enableFIFOfull;

int LoadUSF(char *fn) ;

#endif

