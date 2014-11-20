#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>

#include "usf.h"
#include "cpu.h"
#include "memory.h"
#include "audio.h"

unsigned long AiReadLength(void);
void AiLenChanged(unsigned long length);


#endif
