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
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <sys/mman.h>

#include "main.h"
#include "cpu.h"
#include "memory.h"
#include "x86.h"
#include "audio.h"
#include "rsp.h"
#include "usf.h"
#include "dma.h"
#include "exception.h"
#include "tlb.h"
#include "pif.h"
#include "registers.h"

uintptr_t *TLB_Map = 0;
uint8_t * MemChunk = 0;
uint32_t RdramSize = 0x800000;
uint8_t * N64MEM = 0, * RDRAM = 0, * DMEM = 0, * IMEM = 0, * ROMPages[0x400], * savestatespace = 0, * NOMEM = 0;
void ** JumpTable = 0, ** DelaySlotTable = 0;
uint8_t * RecompCode = 0;

uint32_t MemoryState = 0;

#define PAGE_SIZE   4096
void *malloc_exec(uint32_t bytes)
{
    void *ptr = NULL;

    ptr = mmap(0,bytes,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, 0, 0);

    return ptr;

}

int32_t Allocate_Memory ( void )
{
    //uint32_t i = 0;
    //RdramSize = 0x800000;

    // Allocate the N64MEM and TLB_Map so that they are in each others 4GB range
    // Also put the registers there :)


    // the mmap technique works craptacular when the regions don't overlay

    MemChunk = mmap(NULL, 0x100000 * sizeof(uintptr_t) + 0x1D000 + RdramSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    TLB_Map = (uintptr_t*)MemChunk;
    if (TLB_Map == NULL)
    {
        return 0;
    }

    memset(TLB_Map, 0, 0x100000 * sizeof(uintptr_t) + 0x10000);

    N64MEM = mmap((void*)((uintptr_t)MemChunk + 0x100000 * sizeof(uintptr_t) + 0x10000), 0xD000 + RdramSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, 0, 0);
    if(N64MEM == NULL)
    {
        DisplayError("Failed to allocate N64MEM");
        return 0;
    }

    memset(N64MEM, 0, RdramSize);

    NOMEM = mmap((void*)((uintptr_t)N64MEM + RdramSize), 0xD000, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, 0, 0);

    if(RdramSize == 0x400000)
    {
        //  munmap(N64MEM + 0x400000, 0x400000);
    }

    Registers = (N64_REGISTERS *)((uintptr_t)MemChunk + 0x100000 * sizeof(uintptr_t));
    TLBLoadAddress = (uint32_t *)((uintptr_t)Registers + 0x500);
    Timers = (SYSTEM_TIMERS*)(TLBLoadAddress + 4);
    WaitMode = (uint32_t *)(Timers + sizeof(SYSTEM_TIMERS));
    CPU_Action = (CPU_ACTION *)(WaitMode + 4);
    RSP_GPR = (REGISTER32 *)(CPU_Action + sizeof(CPU_ACTION));
    DMEM = (uint8_t *)(RSP_GPR + (32 * 8));
    RSP_ACCUM = (REGISTER *)(DMEM + 0x2000);
    RSP_Vect = (VECTOR *)((char*)RSP_ACCUM + (sizeof(REGISTER)*32));


    if(CPU_Type != CPU_Interpreter)
    {
        JumpTable = (void **)malloc(0x200000 * sizeof(uintptr_t));

        if( JumpTable == NULL )
        {
            return 0;
        }

        memset(JumpTable, 0, 0x200000 * sizeof(uintptr_t));

        RecompCode = malloc_exec(NormalCompileBufferSize);

        memset(RecompCode, 0xcc, NormalCompileBufferSize);  // fill with Breakpoints

        DelaySlotTable = (void **) malloc((0x1000000) >> 0xA);
        if( DelaySlotTable == NULL )
        {
            return 0;
        }

        memset(DelaySlotTable, 0, ((0x1000000) >> 0xA));

    }
    else
    {
        JumpTable = NULL;
        RecompCode = NULL;
        DelaySlotTable = NULL;
    }
    RDRAM = (uint8_t *)(N64MEM);
    IMEM  = DMEM + 0x1000;

    MemoryState = 1;

    return 1;
}

int PreAllocate_Memory(void)
{
    int i = 0;

    // Moved the savestate allocation here :)  (for better management later)
    savestatespace = malloc(0x80275C);

    if(savestatespace == 0)
    {
        DisplayError("Allocation of savestatespace failed.");
        return 0;
    }
    memset(savestatespace, 0, 0x80275C);

    for (i = 0; i < 0x400; i++)
    {
        ROMPages[i] = 0;
    }

    return 1;
}

void Release_Memory ( void )
{
    uint32_t i;

    for (i = 0; i < 0x400; i++)
    {
        if (ROMPages[i])
        {
            free(ROMPages[i]);
            ROMPages[i] = 0;
        }
    }
    printf("Freeing memory\n");

    MemoryState = 0;

    if (MemChunk != 0)
    {
        munmap(MemChunk, 0x100000 * sizeof(uintptr_t));//+ 0x1D000 + RdramSize;
//                                                         ^   unused value?  ^
        MemChunk=0;
    }
    if (N64MEM != 0)
    {
        munmap(N64MEM, RdramSize);
        N64MEM=0;
    }
    if (NOMEM != 0)
    {
        munmap(NOMEM, 0xD000);
        NOMEM=0;
    }

    if (DelaySlotTable != NULL)
    {
        free( DelaySlotTable);
        DelaySlotTable=NULL;
    }
    if (JumpTable != NULL)
    {
        free( JumpTable);
        JumpTable=NULL;
    }
    if (RecompCode != NULL)
    {
        munmap( RecompCode, NormalCompileBufferSize);
        RecompCode=NULL;
    }
    if (RSPRecompCode != NULL)
    {
        munmap( RSPRecompCode, RSP_RECOMPMEM_SIZE + RSP_SECRECOMPMEM_SIZE);
        RSPRecompCode=NULL;
    }

    if (RSPJumpTables != NULL)
    {
        free( RSPJumpTables);
        RSPJumpTables=NULL;
    }
    if (JumpTable != NULL)
    {
        free( JumpTable);
        JumpTable=NULL;
    }


    if(savestatespace)
    {
        free(savestatespace);
    }
    savestatespace = NULL;
}

#ifndef  __USE_GNU
#define __USE_GNU
#endif
#include <sys/ucontext.h>

static struct sigaction act;
static struct sigaction oact;
static sigset_t sset;
void sig_handler(int signo, siginfo_t * info, ucontext_t * context);

void InitExceptionHandler(void)
{

    sigset_t blockset;

    sigemptyset(&blockset);
    sigaddset(&blockset, SIGSEGV);

    sigprocmask(SIG_UNBLOCK, &blockset, NULL);

    sigemptyset(&sset);
    sigaddset(&sset, SIGSEGV);

    act.sa_flags = SA_SIGINFO ;
    act.sa_mask = sset;
    act.sa_sigaction = (void (*)(int, siginfo_t*, void*)) sig_handler;

    if(sigaction(SIGSEGV, (const struct sigaction *) &act, &oact))
    {
        printf("error setting up exception handler\n");
    }
}

int r4300i_CPU_MemoryFilter64_2( uintptr_t MemAddress, ucontext_t * context);

void sig_handler(int signo, siginfo_t * info, ucontext_t * context)
{
    int i = 0;

    if(signo==SIGSEGV)
    {
        uintptr_t MemAddress = ((char *)info->si_addr - (char *)N64MEM);

        /*
           A Hack to fix some crappy GCC thing when R15 gets overwritten.
           R15 should _never_ be overwritten. >:(
        */
#ifdef USEX64
        if(context->uc_mcontext.gregs[REG_R15] != (uintptr_t)TLB_Map)
        {
            context->uc_mcontext.gregs[REG_R15] = (uintptr_t)TLB_Map;
            return;
        }
#endif
        i = r4300i_CPU_MemoryFilter64_2(MemAddress,context);
        if(i==0)
        {
            return;
        }
    }

    return;
}

static int32_t CONV_REG64(int32_t dest_reg)
{
    switch(dest_reg)
    {
    case 0:
        return 13;
        break;
    case 1:
        return 14;
        break;
    case 2:
        return 12;
        break;
    case 3:
        return 11;
        break;
    case 6:
        return 9;
        break;
    case 7:
        return 8;
        break;
    case 8:
        return 0;
        break;
    case 9:
        return 1;
        break;
    case 10:
        return 2;
        break;
    case 11:
        return 3;
        break;
    case 12:
        return 4;
        break;
    case 13:
        return 5;
        break;
    case 14:
        return 6;
        break;
    case 15:
        return 7;
        break;
    default:
        asm("int $3");
        return 255;
        break;
    }
}


#ifdef __LP64__
int r4300i_CPU_MemoryFilter64_2( uintptr_t MemAddress, ucontext_t * context)
{
    uint8_t * ip = (uint8_t *)context->uc_mcontext.gregs[REG_RIP];

    if(MemAddress == 0)
    {
        return 1;
    }

    if((*ip & 0x40) && (*(ip+1) == 0xf) && (*(ip+2) == 0xb7))
    {
        uint8_t dest_reg = (*(ip+3) % 0x40) / 8;
        uint32_t half = 0;
        if(*ip & 4)
        {
            dest_reg += 8;
        }
        r4300i_LH_NonMemory(MemAddress, &half, 1);

        context->uc_mcontext.gregs[CONV_REG64(dest_reg)] = (int32_t)half;


        context->uc_mcontext.gregs[REG_RIP]+=4;

        if((*(ip+3) & 0x7)==4)
        {
            context->uc_mcontext.gregs[REG_RIP]++;
        }

        if((*(ip+3) & 0xC0) == 0x80)
        {
            context->uc_mcontext.gregs[REG_RIP]+=4;
        }
        else if((*(ip+3) & 0xC0) == 0x40)
        {
            context->uc_mcontext.gregs[REG_RIP]+=1;
        }

        return 0;

    }
    else if((*ip & 0x40) && (*(ip+1) ==0x89))     // MOV [Rxx + Rxx], Exx
    {
        uint8_t dest_reg = (*(ip+2) % 0x40) / 8;
        uint64_t dest = 0;

        if(*ip & 4)
        {
            dest_reg += 8;
        }

        dest = context->uc_mcontext.gregs[CONV_REG64(dest_reg)];

        r4300i_SW_NonMemory(MemAddress, dest);

        if((*(ip+2) & 0x7)==4)
        {
            context->uc_mcontext.gregs[REG_RIP]+=4;
        }
        else
        {
            context->uc_mcontext.gregs[REG_RIP]+=3;
        }

        if((*(ip+2) & 0xC0) == 0x80)
        {
            context->uc_mcontext.gregs[REG_RIP]+=4;
        }
        else if((*(ip+2) & 0xC0) == 0x40)
        {
            context->uc_mcontext.gregs[REG_RIP]+=1;
        }

        return 0;

    }
    else if((*ip & 0x40) && (*(ip+1) ==0xC7))     // MOV [Rxx + Rxx], Imm32
    {
        uint32_t imm32 = *(uint32_t*)(ip+4);
        r4300i_SW_NonMemory(MemAddress, imm32);
        context->uc_mcontext.gregs[REG_RIP]+=7;

        // 40 C7 04 07 0F 00 00 00

        //if(*(ip+2)&0x4)
        if((*(ip+2) & 0x7)==4)
        {
            context->uc_mcontext.gregs[REG_RIP]++;
        }
        if((*(ip+2) & 0xC0) == 0x80)
        {
            context->uc_mcontext.gregs[REG_RIP]+=4;
        }
        else if((*(ip+2) & 0xC0) == 0x40)
        {
            context->uc_mcontext.gregs[REG_RIP]+=1;
        }

        return 0;
    }
    else if ((*ip & 0x40) && (*(ip + 1) == 0x8B ))
    {
        uint8_t dest_reg = (*(ip+2) % 0x40) / 8;
        uint32_t word = 0;
        //uint64_t *dest = 0;

        //41 8B BF 30 D0 00 01

        if(*ip & 4)
        {
            dest_reg += 8;
        }


        r4300i_LW_NonMemory(MemAddress, &word);
        context->uc_mcontext.gregs[CONV_REG64(dest_reg)] = word;

        //*dest = 0;
        //if(*(ip+2) & 0x4)
        if((*(ip+2) & 0x7)==4)
        {
            context->uc_mcontext.gregs[REG_RIP]+=4;
        }
        else
        {
            context->uc_mcontext.gregs[REG_RIP]+=3;
        }

        if((*(ip+2) & 0xC0) == 0x80)
        {
            context->uc_mcontext.gregs[REG_RIP]+=4;
        }
        else if((*(ip+2) & 0xC0) == 0x40)
        {
            context->uc_mcontext.gregs[REG_RIP]+=1;
        }


        return 0;

    }

    //asm("int $3");
    return 1;
}

#else

static int32_t CONV_REG(int32_t dest_reg)
{
    switch(dest_reg)
    {
    case 0:
        return 5;
        break;
    case 1:
        return 4;
        break;
    case 2:
        return 3;
        break;
    case 3:
        return 2;
        break;
    case 6:
        return 1;
        break;
    case 7:
        return 0;
        break;
    default:
        asm volatile("int $3");
        return 255;
        break;
    }
}

int r4300i_CPU_MemoryFilter64_2( uintptr_t MemAddress, ucontext_t * context)
{
    uint8_t * ip = (uint8_t *)context->uc_mcontext.gregs[REG_EIP];

    if(MemAddress == 0)
    {
        return 1;
    }

    if((*(ip) == 0xf) && (*(ip+1) == 0xb7))
    {
        uint8_t dest_reg = (*(ip+2) % 0x40) / 8;
        uint32_t half = 0;
        r4300i_LH_NonMemory(MemAddress, &half, 1);

        //((uint32_t*)(&lpEP->ContextRecord->Edi))[CONV_REG(dest_reg)] = (int32_t)half;
        context->uc_mcontext.gregs[CONV_REG64(dest_reg)] = (int32_t)half;

        context->uc_mcontext.gregs[REG_EIP]+=3;

        if((*(ip+2) & 0x7)==4)
        {
            context->uc_mcontext.gregs[REG_EIP]++;
        }
        else if((*(ip+1) & 0x7)==5)
        {
            context->uc_mcontext.gregs[REG_EIP]+=4;
        }


        if((*(ip+2) & 0xC0) == 0x80)
        {
            context->uc_mcontext.gregs[REG_EIP]+=4;
        }
        else if((*(ip+2) & 0xC0) == 0x40)
        {
            context->uc_mcontext.gregs[REG_EIP]+=1;
        }

        return 0;

    }
    else if((*(ip) ==0x89))     // MOV [Rxx + Rxx], Exx
    {
        uint8_t dest_reg = (*(ip+1) % 0x40) / 8;
        uint32_t dest = 0;

        dest = context->uc_mcontext.gregs[CONV_REG64(dest_reg)];

        r4300i_SW_NonMemory(MemAddress, dest);

        if((*(ip+1) & 0x7)==4)
        {
            context->uc_mcontext.gregs[REG_EIP]+=3;
        }
        else
        {
            context->uc_mcontext.gregs[REG_EIP]+=2;
        }

        if((*(ip+1) & 0xC0) == 0x80)
        {
            context->uc_mcontext.gregs[REG_EIP]+=4;
        }
        else if((*(ip+1) & 0xC0) == 0x40)
        {
            context->uc_mcontext.gregs[REG_EIP]+=1;
        }

        return 0;

    }
    else if((*(ip) ==0xC7))     // MOV [Rxx + Rxx], Imm32
    {
        uint32_t imm32 = *(uint32_t*)(ip+2);
        r4300i_SW_NonMemory(MemAddress, imm32);
        context->uc_mcontext.gregs[REG_EIP]+=6;


        // 40 C7 04 07 0F 00 00 00

        //if(*(ip+2)&0x4)
        if((*(ip+1) & 0x7)==4)
        {
            context->uc_mcontext.gregs[REG_EIP]++;
        }
        if((*(ip+1) & 0xC0) == 0x80)
        {
            context->uc_mcontext.gregs[REG_EIP]+=4;
        }
        else if((*(ip+1) & 0xC0) == 0x40)
        {
            context->uc_mcontext.gregs[REG_EIP]+=1;
        }

        return 0;
    }
    else if (*ip == 0x8B )
    {
        uint8_t dest_reg = CONV_REG((*(ip+1) % 0x40) / 8);
        uint32_t word = 0;
        uint32_t *dest = 0;

        dest = (uint32_t *)context->uc_mcontext.gregs[CONV_REG64(dest_reg)];
        r4300i_LW_NonMemory(MemAddress, &word);
        *dest = word;
        //*dest = 0;
        //if(*(ip+2) & 0x4)
        if((*(ip+1) & 0x7)==4)
        {
            context->uc_mcontext.gregs[REG_EIP]+=3;
        }
        else if((*(ip+1) & 0x7)==5)
        {
            context->uc_mcontext.gregs[REG_EIP]+=6;
        }
        else
        {
            context->uc_mcontext.gregs[REG_EIP]+=2;
        }

        if((*(ip+1) & 0xC0) == 0x80)
        {
            context->uc_mcontext.gregs[REG_EIP]+=4;
        }
        else if((*(ip+1) & 0xC0) == 0x40)
        {
            context->uc_mcontext.gregs[REG_EIP]+=1;
        }


        return 0;

    }

    return 1;

}


#endif
