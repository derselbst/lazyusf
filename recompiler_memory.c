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

uint8_t * RecompPos = 0;

uint32_t TempValue = 0;

void Compile_LB ( int32_t Reg, uint32_t addr, uint32_t SignExtend )
{
    uintptr_t Addr = addr;
    if (!TranslateVaddr(&Addr))
    {
        MoveConstToX86reg(0,Reg);
        return;
    }

    switch (Addr & 0xFFF00000)
    {
    case 0x00000000:
    case 0x00100000:
    case 0x00200000:
    case 0x00300000:
    case 0x00400000:
    case 0x00500000:
    case 0x00600000:
    case 0x00700000:
    case 0x10000000:

        if (SignExtend)
        {
            MoveSxVariableToX86regByte(Addr + N64MEM,Reg);
        }
        else
        {
            MoveZxVariableToX86regByte(Addr + N64MEM,Reg);
        }
        break;
    default:
        MoveConstToX86reg(0,Reg);
    }
}

void Compile_LH ( int32_t Reg, uint32_t addr, uint32_t SignExtend)
{
    uintptr_t Addr = addr;
    if (!TranslateVaddr(&Addr))
    {
        MoveConstToX86reg(0,Reg);
        return;
    }

    switch (Addr & 0xFFF00000)
    {
    case 0x00000000:
    case 0x00100000:
    case 0x00200000:
    case 0x00300000:
    case 0x00400000:
    case 0x00500000:
    case 0x00600000:
    case 0x00700000:
    case 0x10000000:

        if (SignExtend)
        {
            MoveSxVariableToX86regHalf(Addr + N64MEM,Reg);
        }
        else
        {
            MoveZxVariableToX86regHalf(Addr + N64MEM,Reg);
        }
        break;
    default:
        MoveConstToX86reg(0,Reg);
    }
}

void Compile_LW ( int32_t Reg, uint32_t addr )
{
    uintptr_t Addr = addr;
    if (!TranslateVaddr(&Addr))
    {
        MoveConstToX86reg(0,Reg);
    }

    switch (Addr & 0xFFF00000)
    {
    case 0x00000000:
    case 0x00100000:
    case 0x00200000:
    case 0x00300000:
    case 0x00400000:
    case 0x00500000:
    case 0x00600000:
    case 0x00700000:
    case 0x10000000:
        MoveVariableToX86reg(Addr + N64MEM,Reg);
        break;
    case 0x04000000:
        if (Addr < 0x04002000)
        {
            MoveVariableToX86reg(Addr + N64MEM,Reg);
            break;
        }
        switch (Addr)
        {
        case 0x04040010:
            MoveVariableToX86reg(&SP_STATUS_REG,Reg);
            break;
        case 0x04040014:
            MoveVariableToX86reg(&SP_DMA_FULL_REG,Reg);
            break;
        case 0x04040018:
            MoveVariableToX86reg(&SP_DMA_BUSY_REG,Reg);
            break;
        case 0x04080000:
            MoveVariableToX86reg(&SP_PC_REG,Reg);
            break;
        default:
            MoveConstToX86reg(0,Reg);
            break;
        }
        break;
    case 0x04100000:
        MoveVariableToX86reg(Addr + N64MEM,Reg);
        break;
    case 0x04300000:
        switch (Addr)
        {
        case 0x04300000:
            MoveVariableToX86reg(&MI_MODE_REG,Reg);
            break;
        case 0x04300004:
            MoveVariableToX86reg(&MI_VERSION_REG,Reg);
            break;
        case 0x04300008:
            MoveVariableToX86reg(&MI_INTR_REG,Reg);
            break;
        case 0x0430000C:
            MoveVariableToX86reg(&MI_INTR_MASK_REG,Reg);
            break;
        default:
            MoveConstToX86reg(0,Reg);
            break;
        }
        break;
    case 0x04400000:
        switch (Addr)
        {
        case 0x04400010:
            Pushad();
            Call_Direct(&UpdateCurrentHalfLine);
            Popad();
            MoveVariableToX86reg(&HalfLine,Reg);
            break;
        default:
            MoveConstToX86reg(0,Reg);
            break;
        }
        break;
    case 0x04500000: /* AI registers */
        switch (Addr)
        {
        case 0x04500004:
            Pushad();
            Call_Direct(AiReadLength);
            MoveX86regToVariable(x86_EAX,&TempValue);
            Popad();
            MoveVariableToX86reg(&TempValue,Reg);
            break;
        case 0x0450000C:
            MoveVariableToX86reg(&AI_STATUS_REG,Reg);
            break;
        case 0x04500010:
            MoveVariableToX86reg(&AI_DACRATE_REG,Reg);
            break;
        default:
            MoveConstToX86reg(0,Reg);
            break;
        }
        break;
    case 0x04600000:
        switch (Addr)
        {
        case 0x04600010:
            MoveVariableToX86reg(&PI_STATUS_REG,Reg);
            break;
        case 0x04600014:
            MoveVariableToX86reg(&PI_DOMAIN1_REG,Reg);
            break;
        case 0x04600018:
            MoveVariableToX86reg(&PI_BSD_DOM1_PWD_REG,Reg);
            break;
        case 0x0460001C:
            MoveVariableToX86reg(&PI_BSD_DOM1_PGS_REG,Reg);
            break;
        case 0x04600020:
            MoveVariableToX86reg(&PI_BSD_DOM1_RLS_REG,Reg);
            break;
        case 0x04600024:
            MoveVariableToX86reg(&PI_DOMAIN2_REG,Reg);
            break;
        case 0x04600028:
            MoveVariableToX86reg(&PI_BSD_DOM2_PWD_REG,Reg);
            break;
        case 0x0460002C:
            MoveVariableToX86reg(&PI_BSD_DOM2_PGS_REG,Reg);
            break;
        case 0x04600030:
            MoveVariableToX86reg(&PI_BSD_DOM2_RLS_REG,Reg);
            break;
        default:
            MoveConstToX86reg(0,Reg);
            break;
        }
        break;
    case 0x04700000:
        switch (Addr)
        {
        case 0x0470000C:
            MoveVariableToX86reg(&RI_SELECT_REG,Reg);
            break;
        case 0x04700010:
            MoveVariableToX86reg(&RI_REFRESH_REG,Reg);
            break;
        default:
            MoveConstToX86reg(0,Reg);
            break;
        }
        break;
    case 0x04800000:
        switch (Addr)
        {
        case 0x04800018:
            MoveVariableToX86reg(&SI_STATUS_REG,Reg);
            break;
        default:
            MoveConstToX86reg(0,Reg);
            break;
        }
        break;
    case 0x1FC00000:
        MoveVariableToX86reg(Addr + N64MEM,Reg);
        break;
    default:
        MoveConstToX86reg(((Addr & 0xFFFF) << 16) | (Addr & 0xFFFF),Reg);
    }
}

void Compile_SB_Const ( uint8_t Value, uint32_t addr )
{
    uintptr_t Addr = addr;
    if (!TranslateVaddr(&Addr))
    {
        return;
    }

    switch (Addr & 0xFFF00000)
    {
    case 0x00000000:
    case 0x00100000:
    case 0x00200000:
    case 0x00300000:
    case 0x00400000:
    case 0x00500000:
    case 0x00600000:
    case 0x00700000:
        MoveConstByteToVariable(Value,Addr + N64MEM);
        break;
    }
}

void Compile_SB_Register ( int32_t x86Reg, uint32_t addr )
{
    uintptr_t Addr = addr;
    if (!TranslateVaddr(&Addr))
    {
        return;
    }

    switch (Addr & 0xFFF00000)
    {
    case 0x00000000:
    case 0x00100000:
    case 0x00200000:
    case 0x00300000:
    case 0x00400000:
    case 0x00500000:
    case 0x00600000:
    case 0x00700000:
        MoveX86regByteToVariable(x86Reg,Addr + N64MEM);
        break;
    }
}

void Compile_SH_Const ( uint16_t Value, uint32_t addr )
{
    uintptr_t Addr = addr;
    if (!TranslateVaddr(&Addr))
    {
        return;
    }

    switch (Addr & 0xFFF00000)
    {
    case 0x00000000:
    case 0x00100000:
    case 0x00200000:
    case 0x00300000:
    case 0x00400000:
    case 0x00500000:
    case 0x00600000:
    case 0x00700000:
        MoveConstHalfToVariable(Value,Addr + N64MEM);
        break;
    }
}

void Compile_SH_Register ( int32_t x86Reg, uint32_t addr )
{
    uintptr_t Addr = addr;
    if (!TranslateVaddr(&Addr))
    {
        return;
    }

    switch (Addr & 0xFFF00000)
    {
    case 0x00000000:
    case 0x00100000:
    case 0x00200000:
    case 0x00300000:
    case 0x00400000:
    case 0x00500000:
    case 0x00600000:
    case 0x00700000:
        MoveX86regHalfToVariable(x86Reg,Addr + N64MEM);
        break;
    }
}

void Compile_SW_Const ( uint32_t Value, uint32_t addr )
{
    uintptr_t Addr = addr;
    if (!TranslateVaddr(&Addr))
    {
        return;
    }

    switch (Addr & 0xFFF00000)
    {
    case 0x00000000:
    case 0x00100000:
    case 0x00200000:
    case 0x00300000:
    case 0x00400000:
    case 0x00500000:
    case 0x00600000:
    case 0x00700000:
        MoveConstToVariable(Value,Addr + N64MEM);
        break;
    case 0x03F00000:
        switch (Addr)
        {
        case 0x03F00000:
            MoveConstToVariable(Value,&RDRAM_CONFIG_REG);
            break;
        case 0x03F00004:
            MoveConstToVariable(Value,&RDRAM_DEVICE_ID_REG);
            break;
        case 0x03F00008:
            MoveConstToVariable(Value,&RDRAM_DELAY_REG);
            break;
        case 0x03F0000C:
            MoveConstToVariable(Value,&RDRAM_MODE_REG);
            break;
        case 0x03F00010:
            MoveConstToVariable(Value,&RDRAM_REF_INTERVAL_REG);
            break;
        case 0x03F00014:
            MoveConstToVariable(Value,&RDRAM_REF_ROW_REG);
            break;
        case 0x03F00018:
            MoveConstToVariable(Value,&RDRAM_RAS_INTERVAL_REG);
            break;
        case 0x03F0001C:
            MoveConstToVariable(Value,&RDRAM_MIN_INTERVAL_REG);
            break;
        case 0x03F00020:
            MoveConstToVariable(Value,&RDRAM_ADDR_SELECT_REG);
            break;
        case 0x03F00024:
            MoveConstToVariable(Value,&RDRAM_DEVICE_MANUF_REG);
            break;
        case 0x03F04004:
            break;
        case 0x03F08004:
            break;
        case 0x03F80004:
            break;
        case 0x03F80008:
            break;
        case 0x03F8000C:
            break;
        case 0x03F80014:
            break;
        }
        break;
    case 0x04000000:
        if (Addr < 0x04002000)
        {
            MoveConstToVariable(Value,Addr + N64MEM);
            break;
        }
        switch (Addr)
        {
        case 0x04040000:
            MoveConstToVariable(Value,&SP_MEM_ADDR_REG);
            break;
        case 0x04040004:
            MoveConstToVariable(Value,&SP_DRAM_ADDR_REG);
            break;
        case 0x04040008:
            MoveConstToVariable(Value,&SP_RD_LEN_REG);
            Pushad();
            Call_Direct(&SP_DMA_READ);
            Popad();
            break;
        case 0x04040010:
        {
            uint32_t ModValue = 0;
            if ( ( Value & SP_CLR_HALT ) != 0 )
            {
                ModValue |= SP_STATUS_HALT;
            }
            if ( ( Value & SP_CLR_BROKE ) != 0 )
            {
                ModValue |= SP_STATUS_BROKE;
            }
            if ( ( Value & SP_CLR_SSTEP ) != 0 )
            {
                ModValue |= SP_STATUS_SSTEP;
            }
            if ( ( Value & SP_CLR_INTR_BREAK ) != 0 )
            {
                ModValue |= SP_STATUS_INTR_BREAK;
            }
            if ( ( Value & SP_CLR_SIG0 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG0;
            }
            if ( ( Value & SP_CLR_SIG1 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG1;
            }
            if ( ( Value & SP_CLR_SIG2 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG2;
            }
            if ( ( Value & SP_CLR_SIG3 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG3;
            }
            if ( ( Value & SP_CLR_SIG4 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG4;
            }
            if ( ( Value & SP_CLR_SIG5 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG5;
            }
            if ( ( Value & SP_CLR_SIG6 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG6;
            }
            if ( ( Value & SP_CLR_SIG7 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG7;
            }

            if (ModValue != 0)
            {
                AndConstToVariable(~ModValue,&SP_STATUS_REG);
            }

            ModValue = 0;
            if ( ( Value & SP_SET_HALT ) != 0 )
            {
                ModValue |= SP_STATUS_HALT;
            }
            if ( ( Value & SP_SET_SSTEP ) != 0 )
            {
                ModValue |= SP_STATUS_SSTEP;
            }
            if ( ( Value & SP_SET_INTR_BREAK ) != 0)
            {
                ModValue |= SP_STATUS_INTR_BREAK;
            }
            if ( ( Value & SP_SET_SIG0 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG0;
            }
            if ( ( Value & SP_SET_SIG1 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG1;
            }
            if ( ( Value & SP_SET_SIG2 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG2;
            }
            if ( ( Value & SP_SET_SIG3 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG3;
            }
            if ( ( Value & SP_SET_SIG4 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG4;
            }
            if ( ( Value & SP_SET_SIG5 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG5;
            }
            if ( ( Value & SP_SET_SIG6 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG6;
            }
            if ( ( Value & SP_SET_SIG7 ) != 0 )
            {
                ModValue |= SP_STATUS_SIG7;
            }
            if (ModValue != 0)
            {
                OrConstToVariable(ModValue,&SP_STATUS_REG);
            }

            if ( ( Value & SP_CLR_INTR ) != 0)
            {
                AndConstToVariable(~MI_INTR_SP,&MI_INTR_REG);
                Pushad();
                Call_Direct(RunRsp);
                Call_Direct(CheckInterrupts);
                Popad();
            }
            else
            {
                Pushad();
                Call_Direct(RunRsp);
                Popad();
            }
        }
        break;
        case 0x0404001C:
            MoveConstToVariable(0,&SP_SEMAPHORE_REG);
            break;
        case 0x04080000:
            MoveConstToVariable(Value & 0xFFC,&SP_PC_REG);
            break;
        }
        break;
    case 0x04300000:
        switch (Addr)
        {
        case 0x04300000:
        {
            uint32_t ModValue = 0x7F;
            if ( ( Value & MI_CLR_INIT ) != 0 )
            {
                ModValue |= MI_MODE_INIT;
            }
            if ( ( Value & MI_CLR_EBUS ) != 0 )
            {
                ModValue |= MI_MODE_EBUS;
            }
            if ( ( Value & MI_CLR_RDRAM ) != 0 )
            {
                ModValue |= MI_MODE_RDRAM;
            }
            if (ModValue != 0)
            {
                AndConstToVariable(~ModValue,&MI_MODE_REG);
            }

            ModValue = (Value & 0x7F);
            if ( ( Value & MI_SET_INIT ) != 0 )
            {
                ModValue |= MI_MODE_INIT;
            }
            if ( ( Value & MI_SET_EBUS ) != 0 )
            {
                ModValue |= MI_MODE_EBUS;
            }
            if ( ( Value & MI_SET_RDRAM ) != 0 )
            {
                ModValue |= MI_MODE_RDRAM;
            }
            if (ModValue != 0)
            {
                OrConstToVariable(ModValue,&MI_MODE_REG);
            }
            if ( ( Value & MI_CLR_DP_INTR ) != 0 )
            {
                AndConstToVariable(~MI_INTR_DP,&MI_INTR_REG);
            }
        }
        break;
        case 0x0430000C:
        {
            uint32_t ModValue;
            ModValue = 0;
            if ( ( Value & MI_INTR_MASK_CLR_SP ) != 0 )
            {
                ModValue |= MI_INTR_MASK_SP;
            }
            if ( ( Value & MI_INTR_MASK_CLR_SI ) != 0 )
            {
                ModValue |= MI_INTR_MASK_SI;
            }
            if ( ( Value & MI_INTR_MASK_CLR_AI ) != 0 )
            {
                ModValue |= MI_INTR_MASK_AI;
            }
            if ( ( Value & MI_INTR_MASK_CLR_VI ) != 0 )
            {
                ModValue |= MI_INTR_MASK_VI;
            }
            if ( ( Value & MI_INTR_MASK_CLR_PI ) != 0 )
            {
                ModValue |= MI_INTR_MASK_PI;
            }
            if ( ( Value & MI_INTR_MASK_CLR_DP ) != 0 )
            {
                ModValue |= MI_INTR_MASK_DP;
            }
            if (ModValue != 0)
            {
                AndConstToVariable(~ModValue,&MI_INTR_MASK_REG);
            }

            ModValue = 0;
            if ( ( Value & MI_INTR_MASK_SET_SP ) != 0 )
            {
                ModValue |= MI_INTR_MASK_SP;
            }
            if ( ( Value & MI_INTR_MASK_SET_SI ) != 0 )
            {
                ModValue |= MI_INTR_MASK_SI;
            }
            if ( ( Value & MI_INTR_MASK_SET_AI ) != 0 )
            {
                ModValue |= MI_INTR_MASK_AI;
            }
            if ( ( Value & MI_INTR_MASK_SET_VI ) != 0 )
            {
                ModValue |= MI_INTR_MASK_VI;
            }
            if ( ( Value & MI_INTR_MASK_SET_PI ) != 0 )
            {
                ModValue |= MI_INTR_MASK_PI;
            }
            if ( ( Value & MI_INTR_MASK_SET_DP ) != 0 )
            {
                ModValue |= MI_INTR_MASK_DP;
            }
            if (ModValue != 0)
            {
                OrConstToVariable(ModValue,&MI_INTR_MASK_REG);
            }
        }
        break;
        }
        break;
    case 0x04400000:
        switch (Addr)
        {
        case 0x04400000:
            MoveConstToVariable(Value,&VI_STATUS_REG);
            break;
        case 0x04400004:
            MoveConstToVariable((Value & 0xFFFFFF),&VI_ORIGIN_REG);
            break;
        case 0x04400008:
            MoveConstToVariable(Value,&VI_WIDTH_REG);
            break;
        case 0x0440000C:
            MoveConstToVariable(Value,&VI_INTR_REG);
            break;
        case 0x04400010:
            AndConstToVariable(~MI_INTR_VI,&MI_INTR_REG);
            Pushad();
            Call_Direct(CheckInterrupts);
            Popad();
            break;
        case 0x04400014:
            MoveConstToVariable(Value,&VI_BURST_REG);
            break;
        case 0x04400018:
            MoveConstToVariable(Value,&VI_V_SYNC_REG);
            break;
        case 0x0440001C:
            MoveConstToVariable(Value,&VI_H_SYNC_REG);
            break;
        case 0x04400020:
            MoveConstToVariable(Value,&VI_LEAP_REG);
            break;
        case 0x04400024:
            MoveConstToVariable(Value,&VI_H_START_REG);
            break;
        case 0x04400028:
            MoveConstToVariable(Value,&VI_V_START_REG);
            break;
        case 0x0440002C:
            MoveConstToVariable(Value,&VI_V_BURST_REG);
            break;
        case 0x04400030:
            MoveConstToVariable(Value,&VI_X_SCALE_REG);
            break;
        case 0x04400034:
            MoveConstToVariable(Value,&VI_Y_SCALE_REG);
            break;
        }
        break;
    case 0x04500000: /* AI registers */
        switch (Addr)
        {
        case 0x04500000:
            MoveConstToVariable(Value,&AI_DRAM_ADDR_REG);
            break;
        case 0x04500004:
            MoveConstToVariable(Value,&AI_LEN_REG);
            Pushad();
            //Pushad();
            //PushImm32(Value);
            Call_Direct(AiLenChanged);
            //AddConstToX86Reg(x86_ESP,8);

            Popad();
            break;
        case 0x04500008:
            MoveConstToVariable((Value & 1),&AI_CONTROL_REG);
            break;
        case 0x0450000C:
            /* Clear Interrupt */
            ;
            AndConstToVariable(~MI_INTR_AI,&MI_INTR_REG);
            AndConstToVariable(~MI_INTR_AI,&AudioIntrReg);
            Pushad();
            Call_Direct(CheckInterrupts);
            Popad();
            break;
        case 0x04500010:
            Addr|=0xa0000000;
            MoveConstToVariable(Value,Addr + N64MEM);
            break;
        case 0x04500014:
            MoveConstToVariable(Value,&AI_BITRATE_REG);
            break;
        }
        break;
    case 0x04600000:
        switch (Addr)
        {
        case 0x04600000:
            MoveConstToVariable(Value,&PI_DRAM_ADDR_REG);
            break;
        case 0x04600004:
            MoveConstToVariable(Value,&PI_CART_ADDR_REG);
            break;
        case 0x04600008:
            MoveConstToVariable(Value,&PI_RD_LEN_REG);
            Pushad();
            Call_Direct(&PI_DMA_READ);
            Popad();
            break;
        case 0x0460000C:
            MoveConstToVariable(Value,&PI_WR_LEN_REG);
            Pushad();
            Call_Direct(&PI_DMA_WRITE);
            Popad();
            break;
        case 0x04600010:
            if ((Value & PI_CLR_INTR) != 0 )
            {
                AndConstToVariable(~MI_INTR_PI,&MI_INTR_REG);
                Pushad();
                Call_Direct(CheckInterrupts);
                Popad();
            }
            break;
        case 0x04600014:
            MoveConstToVariable((Value & 0xFF),&PI_DOMAIN1_REG);
            break;
        case 0x04600018:
            MoveConstToVariable((Value & 0xFF),&PI_BSD_DOM1_PWD_REG);
            break;
        case 0x0460001C:
            MoveConstToVariable((Value & 0xFF),&PI_BSD_DOM1_PGS_REG);
            break;
        case 0x04600020:
            MoveConstToVariable((Value & 0xFF),&PI_BSD_DOM1_RLS_REG);
            break;
        }
        break;
    case 0x04700000:
        switch (Addr)
        {
        case 0x04700000:
            MoveConstToVariable(Value,&RI_MODE_REG);
            break;
        case 0x04700004:
            MoveConstToVariable(Value,&RI_CONFIG_REG);
            break;
        case 0x04700008:
            MoveConstToVariable(Value,&RI_CURRENT_LOAD_REG);
            break;
        case 0x0470000C:
            MoveConstToVariable(Value,&RI_SELECT_REG);
            break;
        }
        break;
    case 0x04800000:
        switch (Addr)
        {
        case 0x04800000:
            MoveConstToVariable(Value,&SI_DRAM_ADDR_REG);
            break;
        case 0x04800004:
            MoveConstToVariable(Value,&SI_PIF_ADDR_RD64B_REG);
            Pushad();
            Call_Direct(&SI_DMA_READ);
            Popad();
            break;
        case 0x04800010:
            MoveConstToVariable(Value,&SI_PIF_ADDR_WR64B_REG);
            Pushad();
            Call_Direct(&SI_DMA_WRITE);
            Popad();
            break;
        case 0x04800018:
            AndConstToVariable(~MI_INTR_SI,&MI_INTR_REG);
            AndConstToVariable(~SI_STATUS_INTERRUPT,&SI_STATUS_REG);
            Pushad();
            Call_Direct(CheckInterrupts);
            Popad();
            break;
        }
        break;
    }
}

void Compile_SW_Register ( int32_t x86Reg, uint32_t addr )
{
    uintptr_t Addr = addr;


    if (!TranslateVaddr(&Addr))
    {
        return;
    }

    switch (Addr & 0xFFF00000)
    {
    case 0x00000000:
    case 0x00100000:
    case 0x00200000:
    case 0x00300000:
    case 0x00400000:
    case 0x00500000:
    case 0x00600000:
    case 0x00700000:
        MoveX86regToVariable(x86Reg,Addr + N64MEM);
        break;
    case 0x04000000:
        switch (Addr)
        {
        case 0x04040000:
            MoveX86regToVariable(x86Reg,&SP_MEM_ADDR_REG);
            break;
        case 0x04040004:
            MoveX86regToVariable(x86Reg,&SP_DRAM_ADDR_REG);
            break;
        case 0x04040008:
            MoveX86regToVariable(x86Reg,&SP_RD_LEN_REG);
            Pushad();
            Call_Direct(&SP_DMA_READ);
            Popad();
            break;
        case 0x0404000C:
            MoveX86regToVariable(x86Reg,&SP_WR_LEN_REG);
            Pushad();
            Call_Direct(&SP_DMA_WRITE);
            Popad();
            break;
        case 0x04040010:
            MoveX86regToVariable(x86Reg,&RegModValue);
            Pushad();
            Call_Direct(ChangeSpStatus);
            Popad();
            break;
        case 0x0404001C:
            MoveConstToVariable(0,&SP_SEMAPHORE_REG);
            break;
        case 0x04080000:
            MoveX86regToVariable(x86Reg,&SP_PC_REG);
            AndConstToVariable(0xFFC,&SP_PC_REG);
            break;
        default:
            if (Addr < 0x04002000)
            {
                MoveX86regToVariable(x86Reg,Addr + N64MEM);
            }
        }
        break;
    case 0x04100000:
        break;
        //MoveX86regToVariable(x86Reg,Addr + N64MEM);
    case 0x04300000:
        switch (Addr)
        {
        case 0x04300000:
            MoveX86regToVariable(x86Reg,&RegModValue);
            Pushad();
            Call_Direct(ChangeMiIntrMask);
            Popad();
            break;
        case 0x0430000C:
            MoveX86regToVariable(x86Reg,&RegModValue);
            Pushad();
            Call_Direct(ChangeMiIntrMask);
            Popad();
            break;
        }
        break;

    case 0x04400000:
        switch (Addr)
        {
        case 0x04400000:
            // inserted in place of compiled stuff
            MoveX86regToVariable(x86Reg,&VI_STATUS_REG);
            break;
        case 0x04400004:
            MoveX86regToVariable(x86Reg,&VI_ORIGIN_REG);
            AndConstToVariable(0xFFFFFF,&VI_ORIGIN_REG);
            break;
        case 0x04400008:
            // inserted in place of compiled stuff
            MoveX86regToVariable(x86Reg,&VI_WIDTH_REG);
            break;
        case 0x0440000C:
            MoveX86regToVariable(x86Reg,&VI_INTR_REG);
            break;
        case 0x04400010:
            AndConstToVariable(~MI_INTR_VI,&MI_INTR_REG);
            Pushad();
            Call_Direct(CheckInterrupts);
            Popad();
            break;
        case 0x04400014:
            MoveX86regToVariable(x86Reg,&VI_BURST_REG);
            break;
        case 0x04400018:
            MoveX86regToVariable(x86Reg,&VI_V_SYNC_REG);
            break;
        case 0x0440001C:
            MoveX86regToVariable(x86Reg,&VI_H_SYNC_REG);
            break;
        case 0x04400020:
            MoveX86regToVariable(x86Reg,&VI_LEAP_REG);
            break;
        case 0x04400024:
            MoveX86regToVariable(x86Reg,&VI_H_START_REG);
            break;
        case 0x04400028:
            MoveX86regToVariable(x86Reg,&VI_V_START_REG);
            break;
        case 0x0440002C:
            MoveX86regToVariable(x86Reg,&VI_V_BURST_REG);
            break;
        case 0x04400030:
            MoveX86regToVariable(x86Reg,&VI_X_SCALE_REG);
            break;
        case 0x04400034:
            MoveX86regToVariable(x86Reg,&VI_Y_SCALE_REG);
            break;
        }
        break;
    case 0x04500000: /* AI registers */
        switch (Addr)
        {
        case 0x04500000:
            MoveX86regToVariable(x86Reg,&AI_DRAM_ADDR_REG);
            break;
        case 0x04500004:
            MoveX86regToVariable(x86Reg,&AI_LEN_REG);
            Pushad();
            //MoveX86RegToX86Reg(x86_ESP, x86_EAX);
            //AndConstToX86Reg(x86_EAX, 8);
            //SubX86RegToX86Reg(x86_ESP,x86_EAX);
            //Push(x86_EAX);
            Call_Direct(AiLenChanged);
            //Pop(x86_EAX);
            //AddX86RegToX86Reg(x86_ESP,x86_EAX);
            Popad();
            break;
        case 0x04500008:
            MoveX86regToVariable(x86Reg,&AI_CONTROL_REG);
            AndConstToVariable(1,&AI_CONTROL_REG);
        case 0x0450000C:
            /* Clear Interrupt */
            ;
            AndConstToVariable(~MI_INTR_AI,&MI_INTR_REG);
            AndConstToVariable(~MI_INTR_AI,&AudioIntrReg);
            Pushad();
            Call_Direct(CheckInterrupts);
            Popad();
            break;
        case 0x04500010:
            MoveX86regToVariable(x86Reg,Addr + N64MEM);
            break;
        default:
            MoveX86regToVariable(x86Reg,Addr + N64MEM);
        }
        break;
    case 0x04600000:
        switch (Addr)
        {
        case 0x04600000:
            MoveX86regToVariable(x86Reg,&PI_DRAM_ADDR_REG);
            break;
        case 0x04600004:
            MoveX86regToVariable(x86Reg,&PI_CART_ADDR_REG);
            break;
        case 0x04600008:
            MoveX86regToVariable(x86Reg,&PI_RD_LEN_REG);
            Pushad();
            Call_Direct(&PI_DMA_READ);
            Popad();
            break;
        case 0x0460000C:
            MoveX86regToVariable(x86Reg,&PI_WR_LEN_REG);
            Pushad();
            Call_Direct(&PI_DMA_WRITE);
            Popad();
            break;
        case 0x04600010:
            AndConstToVariable(~MI_INTR_PI,&MI_INTR_REG);
            Pushad();
            Call_Direct(CheckInterrupts);
            Popad();
            break;
            MoveX86regToVariable(x86Reg,&VI_ORIGIN_REG);
            AndConstToVariable(0xFFFFFF,&VI_ORIGIN_REG);
        case 0x04600014:
            MoveX86regToVariable(x86Reg,&PI_DOMAIN1_REG);
            AndConstToVariable(0xFF,&PI_DOMAIN1_REG);
            break;
        case 0x04600018:
            MoveX86regToVariable(x86Reg,&PI_BSD_DOM1_PWD_REG);
            AndConstToVariable(0xFF,&PI_BSD_DOM1_PWD_REG);
            break;
        case 0x0460001C:
            MoveX86regToVariable(x86Reg,&PI_BSD_DOM1_PGS_REG);
            AndConstToVariable(0xFF,&PI_BSD_DOM1_PGS_REG);
            break;
        case 0x04600020:
            MoveX86regToVariable(x86Reg,&PI_BSD_DOM1_RLS_REG);
            AndConstToVariable(0xFF,&PI_BSD_DOM1_RLS_REG);
            break;
        }
        break;
    case 0x04700000:
        switch (Addr)
        {
        case 0x04700010:
            MoveX86regToVariable(x86Reg,&RI_REFRESH_REG);
            break;
        }
        break;
    case 0x04800000:
        switch (Addr)
        {
        case 0x04800000:
            MoveX86regToVariable(x86Reg,&SI_DRAM_ADDR_REG);
            break;
        case 0x04800004:
            MoveX86regToVariable(x86Reg,&SI_PIF_ADDR_RD64B_REG);
            Pushad();
            Call_Direct(&SI_DMA_READ);
            Popad();
            break;
        case 0x04800010:
            MoveX86regToVariable(x86Reg,&SI_PIF_ADDR_WR64B_REG);
            Pushad();
            Call_Direct(&SI_DMA_WRITE);
            Popad();
            break;
        case 0x04800018:
            AndConstToVariable(~MI_INTR_SI,&MI_INTR_REG);
            AndConstToVariable(~SI_STATUS_INTERRUPT,&SI_STATUS_REG);
            Pushad();
            Call_Direct(CheckInterrupts);
            Popad();
            break;
        }
        break;
    case 0x1FC00000:
        MoveX86regToVariable(x86Reg,Addr + N64MEM);
        break;
    }
}

void ResetRecompCode (void)
{
    uint32_t count;
    RecompPos = RecompCode;
    TargetIndex = 0;

    //Jump Table
    for (count = 0; count < (RdramSize >> 12); count ++ )
    {
        if (N64_Blocks.NoOfRDRamBlocks[count] > 0)
        {
            N64_Blocks.NoOfRDRamBlocks[count] = 0;
            memset(JumpTable + (count << 10),0,0x1000);
            *(DelaySlotTable + count) = NULL;

        }
    }

    if (N64_Blocks.NoOfDMEMBlocks > 0)
    {
        N64_Blocks.NoOfDMEMBlocks = 0;
        memset(JumpTable + (0x04000000 >> 2),0,0x1000);
        *(DelaySlotTable + (0x04000000 >> 12)) = NULL;
    }
    if (N64_Blocks.NoOfIMEMBlocks > 0)
    {
        N64_Blocks.NoOfIMEMBlocks = 0;
        memset(JumpTable + (0x04001000 >> 2),0,0x1000);
        *(DelaySlotTable + (0x04001000 >> 12)) = NULL;
    }
}
