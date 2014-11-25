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
#define LargeCompileBufferSize  0x03200000
#define NormalCompileBufferSize 0x01500000

#define RSP_RECOMPMEM_SIZE      0x400000
#define RSP_SECRECOMPMEM_SIZE   0x200000

extern uint8_t *RecompPos;

/* Recompiler Memory Functions */
void Compile_LB                  ( int32_t Reg, uint32_t Addr, uint32_t SignExtend );
void Compile_LH                  ( int32_t Reg, uint32_t Addr, uint32_t SignExtend );
void Compile_LW                  ( int32_t Reg, uint32_t Addr );
void Compile_SB_Const            ( uint8_t Value, uint32_t Addr );
void Compile_SB_Register         ( int32_t x86Reg, uint32_t Addr );
void Compile_SH_Const            ( uint16_t Value, uint32_t Addr );
void Compile_SH_Register         ( int32_t x86Reg, uint32_t Addr );
void Compile_SW_Const            ( uint32_t Value, uint32_t Addr );
void Compile_SW_Register         ( int32_t x86Reg, uint32_t Addr );
void ResetRecompCode             ( void );
void InitExceptionHandler        ( void );
