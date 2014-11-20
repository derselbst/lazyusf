extern "C"
{
#include "usf.h"
}
#include "audio_hle.h"
#include "memory.h"
#include "audio.h"
// "Mupen64 HLE RSP plugin v0.2 with Azimers code by Hacktarux"


static int audio_ucode_detect ( OSTask_t *task )
{

	if ( LW_PAddr_Imm ( task->ucode_data ) != 0x1 )
	{
		if ( LB_PAddr_Imm ( task->ucode_data ) == 0xF )
			return 4;
		else
			return 3;
	}
	else
	{
		if ( LW_PAddr_Imm ( task->ucode_data + 0x30 ) == 0xF0000F00 )
			return 1;
		else
			return 2;
	}
}

extern "C"
{
	extern void ( *ABI1[0x20] ) ();
}
extern void ( *ABI2[0x20] ) ();
extern void ( *ABI3[0x20] ) ();

void ( *ABI[0x20] ) ();

u32 inst1, inst2;

int audio_ucode ( OSTask_t *task )
{

	unsigned long *p_alist = ( u32* ) PageRAM2 ( task->data_ptr+0x75c );
	unsigned int i;

	switch ( audio_ucode_detect ( task ) )
	{
		case 1: // mario ucode
			memcpy ( ABI, ABI1, sizeof ( ABI[0] ) *0x20 );
			//cprintf("Ucode1\n");
			break;
		case 2: // banjo kazooie ucode
			memcpy ( ABI, ABI2, sizeof ( ABI[0] ) *0x20 );
			//cprintf("Ucode2\n");
			break;
		case 3: // zelda ucode
			memcpy ( ABI, ABI3, sizeof ( ABI[0] ) *0x20 );
			//cprintf("Ucode3\n");
			break;
		default:
		{

			return -1;
		}
	}

	for ( i = 0; i < ( task->data_size/4 ); i += 2 )
	{
		inst1 = LW_PAddr_Imm ( task->data_ptr + ( i*4 ) );
		inst2 = LW_PAddr_Imm ( task->data_ptr + ( ( i+1 ) *4 ) );
		ABI[inst1 >> 24]();
	}

	return 0;
}
