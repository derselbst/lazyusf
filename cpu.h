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
#ifndef _CPU_H_
#define _CPU_H_

#include "opcode.h"

#ifdef __LP64__
#define USEX86
#define EXTREGS
#endif

typedef struct
{
    int32_t DoSomething;
    int32_t CloseCPU;
    int32_t CheckInterrupts;
    int32_t DoInterrupt;
} CPU_ACTION;

#define MaxTimers               3
#define CompareTimer            0
#define ViTimer                 1
#define AiTimer                 2

typedef struct
{
    int32_t NextTimer[MaxTimers];
    int32_t Active[MaxTimers];
    int32_t CurrentTimerType;
    int32_t Timer;
} SYSTEM_TIMERS;

void ChangeCompareTimer ( void );
void ChangeTimer        ( int32_t Type, int32_t Value );
void CheckTimer         ( void );
void CloseCpu           ( void );
int32_t  DelaySlotEffectsCompare ( uint32_t PC, uint32_t Reg1, uint32_t Reg2 );
int32_t  DelaySlotEffectsJump (uint32_t JumpPC);
void DoSomething        ( void );
void InPermLoop         ( void );
void InitiliazeCPUFlags ( void );
void RefreshScreen      ( void );
void RunRsp             ( void );
void StartEmulation     ( void );
void TimerDone          ( void );
void RecompileTimerDone ( void );
void controlfp          (uint32_t mask);
void StartEmulationFromSave ( void * savestate );

#define NORMAL                  0
#define DO_DELAY_SLOT           1
#define DO_END_DELAY_SLOT       2
#define DELAY_SLOT              3
#define END_DELAY_SLOT          4
#define LIKELY_DELAY_SLOT       5
#define JUMP                    6
#define DELAY_SLOT_DONE         7
#define LIKELY_DELAY_SLOT_DONE  8
#define END_BLOCK               9

enum SaveType
{
    Auto,
    Eeprom_4K,
    Eeprom_16K,
    Sram,
    FlashRam
};

extern uint32_t CPU_Type;
extern uint32_t NextInstruction, JumpToLocation, ManualPaused, CPU_Paused, CountPerOp, AudioIntrReg, * WaitMode, CPU_Type;
extern CPU_ACTION * CPU_Action;
extern SYSTEM_TIMERS * Timers;
extern OPCODE Opcode;
extern uint32_t CPURunning;

static uint32_t OLD_VI_V_SYNC_REG/* = 0*/, VI_INTR_TIME/* = 500000*/;
#endif
