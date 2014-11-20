#include "usf.h"
#include "cpu.h"
#include "memory.h"
#include "ops.h"
#include <math.h>
#include <float.h>

#define EXTERNAL_RELEASE

void * R4300i_Opcode[64];
void * R4300i_Special[64];
void * R4300i_Regimm[32];
void * R4300i_CoP0[32];
void * R4300i_CoP0_Function[64];
void * R4300i_CoP1[32];
void * R4300i_CoP1_BC[32];
void * R4300i_CoP1_S[64];
void * R4300i_CoP1_D[64];
void * R4300i_CoP1_W[64];
void * R4300i_CoP1_L[64];

int _isnan(double number) {
	return 0;
}

void  R4300i_opcode_SPECIAL (void) {
	((void ( *)()) R4300i_Special[ Opcode.funct ])();
}

void  R4300i_opcode_REGIMM (void) {
	((void ( *)()) R4300i_Regimm[ Opcode.rt ])();
}

void  R4300i_opcode_COP0 (void) {
	((void ( *)()) R4300i_CoP0[ Opcode.rs ])();
}

void  R4300i_opcode_COP0_CO (void) {
	((void ( *)()) R4300i_CoP0_Function[ Opcode.funct ])();
}

void  R4300i_opcode_COP1 (void) {
	((void ( *)()) R4300i_CoP1[ Opcode.fmt ])();
}

void  R4300i_opcode_COP1_BC (void) {
	((void ( *)()) R4300i_CoP1_BC[ Opcode.ft ])();
}

void  R4300i_opcode_COP1_S (void) {
	//// _controlfp(RoundingModel,_MCW_RC);
	((void ( *)()) R4300i_CoP1_S[ Opcode.funct ])();
}

void  R4300i_opcode_COP1_D (void) {
//	// _controlfp(RoundingModel,_MCW_RC);
	((void ( *)()) R4300i_CoP1_D[ Opcode.funct ])();
}

void  R4300i_opcode_COP1_W (void) {
	((void ( *)()) R4300i_CoP1_W[ Opcode.funct ])();
}

void  R4300i_opcode_COP1_L (void) {
	((void ( *)()) R4300i_CoP1_L[ Opcode.funct ])();
}

void BuildInterpreter (void ) {
	int i = 0;
	for(i = 0; i < 64; i++) {
		R4300i_Opcode[i] = R4300i_UnknownOpcode;
		R4300i_Special[i] = R4300i_UnknownOpcode;
		R4300i_Regimm[i>>1] = R4300i_UnknownOpcode;
		R4300i_CoP0[i>>1] = R4300i_UnknownOpcode;
		R4300i_CoP0_Function[i] = R4300i_UnknownOpcode;
		R4300i_CoP1[i>>1] = R4300i_UnknownOpcode;
		R4300i_CoP1_BC[i>>1] = R4300i_UnknownOpcode;
		R4300i_CoP1_S[i] = R4300i_UnknownOpcode;
		R4300i_CoP1_D[i] = R4300i_UnknownOpcode;
		R4300i_CoP1_W[i] = R4300i_UnknownOpcode;
		R4300i_CoP1_L[i] = R4300i_UnknownOpcode;
	}

	R4300i_Opcode[ 0] = R4300i_opcode_SPECIAL;
	R4300i_Opcode[ 1] = R4300i_opcode_REGIMM;
	R4300i_Opcode[ 2] = r4300i_J;
	R4300i_Opcode[ 3] = r4300i_JAL;
	R4300i_Opcode[ 4] = r4300i_BEQ;
	R4300i_Opcode[ 5] = r4300i_BNE;
	R4300i_Opcode[ 6] = r4300i_BLEZ;
	R4300i_Opcode[ 7] = r4300i_BGTZ;
	R4300i_Opcode[ 8] = r4300i_ADDI;
	R4300i_Opcode[ 9] = r4300i_ADDIU;
	R4300i_Opcode[10] = r4300i_SLTI;
	R4300i_Opcode[11] = r4300i_SLTIU;
	R4300i_Opcode[12] = r4300i_ANDI;
	R4300i_Opcode[13] = r4300i_ORI;
	R4300i_Opcode[14] = r4300i_XORI;
	R4300i_Opcode[15] = r4300i_LUI;
	R4300i_Opcode[16] = R4300i_opcode_COP0;
	R4300i_Opcode[17] = R4300i_opcode_COP1;
	R4300i_Opcode[20] = r4300i_BEQL;
	R4300i_Opcode[21] = r4300i_BNEL;
	R4300i_Opcode[22] = r4300i_BLEZL;
	R4300i_Opcode[23] = r4300i_BGTZL;
	R4300i_Opcode[25] = r4300i_DADDIU;
	R4300i_Opcode[26] = r4300i_LDL;
	R4300i_Opcode[27] = r4300i_LDR;
	R4300i_Opcode[32] = r4300i_LB;
	R4300i_Opcode[33] = r4300i_LH;
	R4300i_Opcode[34] = r4300i_LWL;
	R4300i_Opcode[35] = r4300i_LW;
	R4300i_Opcode[36] = r4300i_LBU;
	R4300i_Opcode[37] = r4300i_LHU;
	R4300i_Opcode[38] = r4300i_LWR;
	R4300i_Opcode[39] = r4300i_LWU;
	R4300i_Opcode[40] = r4300i_SB;
	R4300i_Opcode[41] = r4300i_SH;
	R4300i_Opcode[42] = r4300i_SWL;
	R4300i_Opcode[43] = r4300i_SW;
	R4300i_Opcode[44] = r4300i_SDL;
	R4300i_Opcode[45] = r4300i_SDR;
	R4300i_Opcode[46] = r4300i_SWR;
	R4300i_Opcode[47] = r4300i_CACHE;
	R4300i_Opcode[48] = r4300i_LL;
	R4300i_Opcode[49] = r4300i_LWC1;
	R4300i_Opcode[53] = r4300i_LDC1;
	R4300i_Opcode[55] = r4300i_LD;
	R4300i_Opcode[56] = r4300i_SC;
	R4300i_Opcode[57] = r4300i_SWC1;
	R4300i_Opcode[61] = r4300i_SDC1;
	R4300i_Opcode[63] = r4300i_SD;

	R4300i_Special[ 0] = r4300i_SPECIAL_SLL;
	R4300i_Special[ 2] = r4300i_SPECIAL_SRL;
	R4300i_Special[ 3] = r4300i_SPECIAL_SRA;
	R4300i_Special[ 4] = r4300i_SPECIAL_SLLV;
	R4300i_Special[ 6] = r4300i_SPECIAL_SRLV;
	R4300i_Special[ 7] = r4300i_SPECIAL_SRAV;
	R4300i_Special[ 8] = r4300i_SPECIAL_JR;
	R4300i_Special[ 9] = r4300i_SPECIAL_JALR;
	R4300i_Special[12] = r4300i_SPECIAL_SYSCALL;
	R4300i_Special[13] = r4300i_SPECIAL_BREAK; //R4300i_UnknownOpcode;
	R4300i_Special[15] = r4300i_SPECIAL_SYNC;
	R4300i_Special[16] = r4300i_SPECIAL_MFHI;
	R4300i_Special[17] = r4300i_SPECIAL_MTHI;
	R4300i_Special[18] = r4300i_SPECIAL_MFLO;
	R4300i_Special[19] = r4300i_SPECIAL_MTLO;
	R4300i_Special[20] = r4300i_SPECIAL_DSLLV;
	R4300i_Special[22] = r4300i_SPECIAL_DSRLV;
	R4300i_Special[23] = r4300i_SPECIAL_DSRAV;
	R4300i_Special[24] = r4300i_SPECIAL_MULT;
	R4300i_Special[25] = r4300i_SPECIAL_MULTU;
	R4300i_Special[26] = r4300i_SPECIAL_DIV;
	R4300i_Special[27] = r4300i_SPECIAL_DIVU;
	R4300i_Special[28] = r4300i_SPECIAL_DMULT;
	R4300i_Special[29] = r4300i_SPECIAL_DMULTU;
	R4300i_Special[30] = r4300i_SPECIAL_DDIV;
	R4300i_Special[31] = r4300i_SPECIAL_DDIVU;
	R4300i_Special[32] = r4300i_SPECIAL_ADD;
	R4300i_Special[33] = r4300i_SPECIAL_ADDU;
	R4300i_Special[34] = r4300i_SPECIAL_SUB;
	R4300i_Special[35] = r4300i_SPECIAL_SUBU;
	R4300i_Special[36] = r4300i_SPECIAL_AND;
	R4300i_Special[37] = r4300i_SPECIAL_OR;
	R4300i_Special[38] = r4300i_SPECIAL_XOR;
	R4300i_Special[39] = r4300i_SPECIAL_NOR;
	R4300i_Special[42] = r4300i_SPECIAL_SLT;
	R4300i_Special[43] = r4300i_SPECIAL_SLTU;
	R4300i_Special[44] = r4300i_SPECIAL_DADD;
	R4300i_Special[45] = r4300i_SPECIAL_DADDU;
	R4300i_Special[46] = r4300i_SPECIAL_DSUB;
	R4300i_Special[47] = r4300i_SPECIAL_DSUBU;
	R4300i_Special[52] = r4300i_SPECIAL_TEQ;
	R4300i_Special[56] = r4300i_SPECIAL_DSLL;
	R4300i_Special[58] = r4300i_SPECIAL_DSRL;
	R4300i_Special[59] = r4300i_SPECIAL_DSRA;
	R4300i_Special[60] = r4300i_SPECIAL_DSLL32;
	R4300i_Special[62] = r4300i_SPECIAL_DSRL32;
	R4300i_Special[63] = r4300i_SPECIAL_DSRA32;

	R4300i_Regimm[ 0] = r4300i_REGIMM_BLTZ;
	R4300i_Regimm[ 1] = r4300i_REGIMM_BGEZ;
	R4300i_Regimm[ 2] = r4300i_REGIMM_BLTZL;
	R4300i_Regimm[ 3] = r4300i_REGIMM_BGEZL;

	R4300i_Regimm[16] = r4300i_REGIMM_BLTZAL;
	R4300i_Regimm[17] = r4300i_REGIMM_BGEZAL;

	R4300i_CoP0[ 0] = r4300i_COP0_MF;
	R4300i_CoP0[ 4] = r4300i_COP0_MT;

	for(i = 16; i < 32; i++)
		R4300i_CoP0[i] = R4300i_opcode_COP0_CO;

	R4300i_CoP0_Function[ 1] = r4300i_COP0_CO_TLBR;
	R4300i_CoP0_Function[ 2] = r4300i_COP0_CO_TLBWI;
	R4300i_CoP0_Function[ 6] = r4300i_COP0_CO_TLBWR;
	R4300i_CoP0_Function[ 8] = r4300i_COP0_CO_TLBP;
	R4300i_CoP0_Function[24] = r4300i_COP0_CO_ERET;

	R4300i_CoP1[ 0] = r4300i_COP1_MF;
	R4300i_CoP1[ 1] = r4300i_COP1_DMF;
	R4300i_CoP1[ 2] = r4300i_COP1_CF;
	R4300i_CoP1[ 4] = r4300i_COP1_MT;
	R4300i_CoP1[ 5] = r4300i_COP1_DMT;
	R4300i_CoP1[ 6] = r4300i_COP1_CT;
	R4300i_CoP1[ 8] = R4300i_opcode_COP1_BC;
	R4300i_CoP1[16] = R4300i_opcode_COP1_S;
	R4300i_CoP1[17] = R4300i_opcode_COP1_D;
	R4300i_CoP1[20] = R4300i_opcode_COP1_W;
	R4300i_CoP1[21] = R4300i_opcode_COP1_L;

	R4300i_CoP1_BC[ 0] = r4300i_COP1_BCF;
	R4300i_CoP1_BC[ 1] = r4300i_COP1_BCT;
	R4300i_CoP1_BC[ 2] = r4300i_COP1_BCFL;
	R4300i_CoP1_BC[ 3] = r4300i_COP1_BCTL;

	R4300i_CoP1_S[ 0] = r4300i_COP1_S_ADD;
	R4300i_CoP1_S[ 1] = r4300i_COP1_S_SUB;
	R4300i_CoP1_S[ 2] = r4300i_COP1_S_MUL;
	R4300i_CoP1_S[ 3] = r4300i_COP1_S_DIV;
	R4300i_CoP1_S[ 4] = r4300i_COP1_S_SQRT;
	R4300i_CoP1_S[ 5] = r4300i_COP1_S_ABS;
	R4300i_CoP1_S[ 6] = r4300i_COP1_S_MOV;
	R4300i_CoP1_S[ 7] = r4300i_COP1_S_NEG;
	R4300i_CoP1_S[ 9] = r4300i_COP1_S_TRUNC_L;
	R4300i_CoP1_S[10] = r4300i_COP1_S_CEIL_L;		//added by Witten
	R4300i_CoP1_S[11] = r4300i_COP1_S_FLOOR_L;		//added by Witten
	R4300i_CoP1_S[12] = r4300i_COP1_S_ROUND_W;
	R4300i_CoP1_S[13] = r4300i_COP1_S_TRUNC_W;
	R4300i_CoP1_S[14] = r4300i_COP1_S_CEIL_W;		//added by Witten
	R4300i_CoP1_S[15] = r4300i_COP1_S_FLOOR_W;
	R4300i_CoP1_S[33] = r4300i_COP1_S_CVT_D;
	R4300i_CoP1_S[36] = r4300i_COP1_S_CVT_W;
	R4300i_CoP1_S[37] = r4300i_COP1_S_CVT_L;

	for(i = 48; i < 64; i++)
		R4300i_CoP1_S[i] = r4300i_COP1_S_CMP;

	R4300i_CoP1_D[ 0] = r4300i_COP1_D_ADD;
	R4300i_CoP1_D[ 1] = r4300i_COP1_D_SUB;
	R4300i_CoP1_D[ 2] = r4300i_COP1_D_MUL;
	R4300i_CoP1_D[ 3] = r4300i_COP1_D_DIV;
	R4300i_CoP1_D[ 4] = r4300i_COP1_D_SQRT;
	R4300i_CoP1_D[ 5] = r4300i_COP1_D_ABS;
	R4300i_CoP1_D[ 6] = r4300i_COP1_D_MOV;
	R4300i_CoP1_D[ 7] = r4300i_COP1_D_NEG;
	R4300i_CoP1_D[ 9] = r4300i_COP1_D_TRUNC_L;		//added by Witten
	R4300i_CoP1_D[10] = r4300i_COP1_D_CEIL_L;		//added by Witten
	R4300i_CoP1_D[11] = r4300i_COP1_D_FLOOR_L;		//added by Witten
	R4300i_CoP1_D[12] = r4300i_COP1_D_ROUND_W;
	R4300i_CoP1_D[13] = r4300i_COP1_D_TRUNC_W;
	R4300i_CoP1_D[14] = r4300i_COP1_D_CEIL_W;		//added by Witten
	R4300i_CoP1_D[15] = r4300i_COP1_D_FLOOR_W;		//added by Witten
	R4300i_CoP1_D[32] = r4300i_COP1_D_CVT_S;
	R4300i_CoP1_D[36] = r4300i_COP1_D_CVT_W;
	R4300i_CoP1_D[37] = r4300i_COP1_D_CVT_L;

	for(i = 48; i < 64; i++)
		R4300i_CoP1_D[i] = r4300i_COP1_D_CMP;

	R4300i_CoP1_W[32] = r4300i_COP1_W_CVT_S;
	R4300i_CoP1_W[33] = r4300i_COP1_W_CVT_D;

	R4300i_CoP1_L[32] = r4300i_COP1_L_CVT_S;
	R4300i_CoP1_L[33] = r4300i_COP1_L_CVT_D;
}

int DelaySlotEffectsCompare (unsigned long PC, unsigned long Reg1, unsigned long Reg2) {
	OPCODE Command;

	if (!LW_VAddr(PC + 4, &Command.Hex)) {
		printf("Failed to load word 2");
		//ExitThread(0);
		return 1;
	}

	switch (Command.op) {
	case R4300i_SPECIAL:
		switch (Command.funct) {
		case R4300i_SPECIAL_SLL:
		case R4300i_SPECIAL_SRL:
		case R4300i_SPECIAL_SRA:
		case R4300i_SPECIAL_SLLV:
		case R4300i_SPECIAL_SRLV:
		case R4300i_SPECIAL_SRAV:
		case R4300i_SPECIAL_MFHI:
		case R4300i_SPECIAL_MTHI:
		case R4300i_SPECIAL_MFLO:
		case R4300i_SPECIAL_MTLO:
		case R4300i_SPECIAL_DSLLV:
		case R4300i_SPECIAL_DSRLV:
		case R4300i_SPECIAL_DSRAV:
		case R4300i_SPECIAL_ADD:
		case R4300i_SPECIAL_ADDU:
		case R4300i_SPECIAL_SUB:
		case R4300i_SPECIAL_SUBU:
		case R4300i_SPECIAL_AND:
		case R4300i_SPECIAL_OR:
		case R4300i_SPECIAL_XOR:
		case R4300i_SPECIAL_NOR:
		case R4300i_SPECIAL_SLT:
		case R4300i_SPECIAL_SLTU:
		case R4300i_SPECIAL_DADD:
		case R4300i_SPECIAL_DADDU:
		case R4300i_SPECIAL_DSUB:
		case R4300i_SPECIAL_DSUBU:
		case R4300i_SPECIAL_DSLL:
		case R4300i_SPECIAL_DSRL:
		case R4300i_SPECIAL_DSRA:
		case R4300i_SPECIAL_DSLL32:
		case R4300i_SPECIAL_DSRL32:
		case R4300i_SPECIAL_DSRA32:
			if (Command.rd == 0) { return 0; }
			if (Command.rd == Reg1) { return 1; }
			if (Command.rd == Reg2) { return 1; }
			break;
		case R4300i_SPECIAL_MULT:
		case R4300i_SPECIAL_MULTU:
		case R4300i_SPECIAL_DIV:
		case R4300i_SPECIAL_DIVU:
		case R4300i_SPECIAL_DMULT:
		case R4300i_SPECIAL_DMULTU:
		case R4300i_SPECIAL_DDIV:
		case R4300i_SPECIAL_DDIVU:
			break;
		default:
			return 1;
		}
		break;
	case R4300i_CP0:
		switch (Command.rs) {
		case R4300i_COP0_MT: break;
		case R4300i_COP0_MF:
			if (Command.rt == 0) { return 0; }
			if (Command.rt == Reg1) { return 1; }
			if (Command.rt == Reg2) { return 1; }
			break;
		default:
			if ( (Command.rs & 0x10 ) != 0 ) {
				switch( Opcode.funct ) {
				case R4300i_COP0_CO_TLBR: break;
				case R4300i_COP0_CO_TLBWI: break;
				case R4300i_COP0_CO_TLBWR: break;
				case R4300i_COP0_CO_TLBP: break;
				default:
					return 1;
				}
			} else {
				return 1;
			}
		}
		break;
	case R4300i_CP1:
		switch (Command.fmt) {
		case R4300i_COP1_MF:
			if (Command.rt == 0) { return 0; }
			if (Command.rt == Reg1) { return 1; }
			if (Command.rt == Reg2) { return 1; }
			break;
		case R4300i_COP1_CF: break;
		case R4300i_COP1_MT: break;
		case R4300i_COP1_CT: break;
		case R4300i_COP1_S: break;
		case R4300i_COP1_D: break;
		case R4300i_COP1_W: break;
		case R4300i_COP1_L: break;
			return 1;
		}
		break;
	case R4300i_ANDI:
	case R4300i_ORI:
	case R4300i_XORI:
	case R4300i_LUI:
	case R4300i_ADDI:
	case R4300i_ADDIU:
	case R4300i_SLTI:
	case R4300i_SLTIU:
	case R4300i_DADDI:
	case R4300i_DADDIU:
	case R4300i_LB:
	case R4300i_LH:
	case R4300i_LW:
	case R4300i_LWL:
	case R4300i_LWR:
	case R4300i_LDL:
	case R4300i_LDR:
	case R4300i_LBU:
	case R4300i_LHU:
	case R4300i_LD:
	case R4300i_LWC1:
	case R4300i_LDC1:
		if (Command.rt == 0) { return 0; }
		if (Command.rt == Reg1) { return 1; }
		if (Command.rt == Reg2) { return 1; }
		break;
	case R4300i_CACHE: break;
	case R4300i_SB: break;
	case R4300i_SH: break;
	case R4300i_SW: break;
	case R4300i_SWR: break;
	case R4300i_SWL: break;
	case R4300i_SWC1: break;
	case R4300i_SDC1: break;
	case R4300i_SD: break;
	default:
		return 1;
	}
	return 0;
}

void TestInterpreterJump (unsigned long PC, unsigned long TargetPC, int Reg1, int Reg2) {
	if (PC != TargetPC) { return; }
	if (DelaySlotEffectsCompare(PC,Reg1,Reg2)) { return; }
	InPermLoop();
}

int RoundingModel = 0;//_RC_NEAR;

#define ADDRESS_ERROR_EXCEPTION(Address,FromRead) \
	DoAddressError(NextInstruction == JUMP,Address,FromRead);\
	NextInstruction = JUMP;\
	JumpToLocation = PROGRAM_COUNTER;\
	return;

//#define TEST_COP1_USABLE_EXCEPTION
#define TEST_COP1_USABLE_EXCEPTION \
	if ((STATUS_REGISTER & STATUS_CU1) == 0) {\
		DoCopUnusableException(NextInstruction == JUMP,1);\
		NextInstruction = JUMP;\
		JumpToLocation = PROGRAM_COUNTER;\
		return;\
	}

#define TLB_READ_EXCEPTION(Address) \
	DoTLBMiss(NextInstruction == JUMP,Address);\
	NextInstruction = JUMP;\
	JumpToLocation = PROGRAM_COUNTER;\
	return;


/************************* OpCode functions *************************/
void  r4300i_J (void) {
	NextInstruction = DELAY_SLOT;
	JumpToLocation = (PROGRAM_COUNTER & 0xF0000000) + (Opcode.target << 2);
	TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,0,0);
}

void  r4300i_JAL (void) {
	NextInstruction = DELAY_SLOT;
	JumpToLocation = (PROGRAM_COUNTER & 0xF0000000) + (Opcode.target << 2);
	TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,0,0);
	state->GPR[31].DW= (long)(PROGRAM_COUNTER + 8);
}

void  r4300i_BEQ (void) {
	NextInstruction = DELAY_SLOT;
	if (state->GPR[Opcode.rs].DW == state->GPR[Opcode.rt].DW) {
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,Opcode.rt);
	} else {
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_BNE (void) {
	NextInstruction = DELAY_SLOT;
	if (state->GPR[Opcode.rs].DW != state->GPR[Opcode.rt].DW) {
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,Opcode.rt);
	} else {
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_BLEZ (void) {
	NextInstruction = DELAY_SLOT;
	if (state->GPR[Opcode.rs].DW <= 0) {
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,0);
	} else {
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_BGTZ (void) {
	NextInstruction = DELAY_SLOT;
	if (state->GPR[Opcode.rs].DW > 0) {
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,0);
	} else {
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_ADDI (void) {
#ifdef Interpreter_StackTest
	if (Opcode.rs == 29 && Opcode.rt == 29) {
		StackValue += (short)Opcode.immediate;
	}
#endif
	if (Opcode.rt == 0) { return; }
	state->GPR[Opcode.rt].DW = (state->GPR[Opcode.rs].W[0] + ((short)Opcode.immediate));
#ifdef Interpreter_StackTest
	if (Opcode.rt == 29 && Opcode.rs != 29) {
		StackValue = state->GPR[Opcode.rt].W[0];
	}
#endif
}

void  r4300i_ADDIU (void) {
#ifdef Interpreter_StackTest
	if (Opcode.rs == 29 && Opcode.rt == 29) {
		StackValue += (short)Opcode.immediate;
	}
#endif
	state->GPR[Opcode.rt].DW = (state->GPR[Opcode.rs].W[0] + ((short)Opcode.immediate));
#ifdef Interpreter_StackTest
	if (Opcode.rt == 29 && Opcode.rs != 29) {
		StackValue = state->GPR[Opcode.rt].W[0];
	}
#endif
}

void  r4300i_SLTI (void) {
	if (state->GPR[Opcode.rs].DW < (long long)((short)Opcode.immediate)) {
		state->GPR[Opcode.rt].DW = 1;
	} else {
		state->GPR[Opcode.rt].DW = 0;
	}
}

void  r4300i_SLTIU (void) {
	int imm32 = (short)Opcode.immediate;
	long long imm64;

	imm64 = imm32;
	state->GPR[Opcode.rt].DW = state->GPR[Opcode.rs].UDW < (unsigned long long)imm64?1:0;
}

void  r4300i_ANDI (void) {
	state->GPR[Opcode.rt].DW = state->GPR[Opcode.rs].DW & Opcode.immediate;
}

void  r4300i_ORI (void) {
	state->GPR[Opcode.rt].DW = state->GPR[Opcode.rs].DW | Opcode.immediate;
}

void  r4300i_XORI (void) {
	state->GPR[Opcode.rt].DW = state->GPR[Opcode.rs].DW ^ Opcode.immediate;
}

void  r4300i_LUI (void) {
	if (Opcode.rt == 0) { return; }
	state->GPR[Opcode.rt].DW = (long)((short)Opcode.offset << 16);
#ifdef Interpreter_StackTest
	if (Opcode.rt == 29) {
		StackValue = state->GPR[Opcode.rt].W[0];
	}
#endif
}

void  r4300i_BEQL (void) {
	if (state->GPR[Opcode.rs].DW == state->GPR[Opcode.rt].DW) {
		NextInstruction = DELAY_SLOT;
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,Opcode.rt);
	} else {
		NextInstruction = JUMP;
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_BNEL (void) {
	if (state->GPR[Opcode.rs].DW != state->GPR[Opcode.rt].DW) {
		NextInstruction = DELAY_SLOT;
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,Opcode.rt);
	} else {
		NextInstruction = JUMP;
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_BLEZL (void) {
	if (state->GPR[Opcode.rs].DW <= 0) {
		NextInstruction = DELAY_SLOT;
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,0);
	} else {
		NextInstruction = JUMP;
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_BGTZL (void) {
	if (state->GPR[Opcode.rs].DW > 0) {
		NextInstruction = DELAY_SLOT;
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,0);
	} else {
		NextInstruction = JUMP;
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_DADDIU (void) {
	state->GPR[Opcode.rt].DW = state->GPR[Opcode.rs].DW + (long long)((short)Opcode.immediate);
}

long long LDL_MASK[8] = { 0ULL,0xFFULL,0xFFFFULL,0xFFFFFFULL,0xFFFFFFFFULL,0xFFFFFFFFFFULL, 0xFFFFFFFFFFFFULL, 0xFFFFFFFFFFFFFFULL };
int LDL_SHIFT[8] = { 0, 8, 16, 24, 32, 40, 48, 56 };

void  r4300i_LDL (void) {
	unsigned long Offset, Address;
	long long Value;

	Address = state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	Offset  = Address & 7;

	if (!LD_VAddr((Address & ~7),&Value)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("LDL TLB: %X",Address);
#endif
		return;
	}
	state->GPR[Opcode.rt].DW = state->GPR[Opcode.rt].DW & LDL_MASK[Offset];
	state->GPR[Opcode.rt].DW += Value << LDL_SHIFT[Offset];
}

long long LDR_MASK[8] = { 0xFFFFFFFFFFFFFF00ULL, 0xFFFFFFFFFFFF0000ULL,
                      0xFFFFFFFFFF000000ULL, 0xFFFFFFFF00000000ULL,
                      0xFFFFFF0000000000ULL, 0xFFFF000000000000ULL,
                      0xFF00000000000000ULL, 0 };
int LDR_SHIFT[8] = { 56, 48, 40, 32, 24, 16, 8, 0 };

void  r4300i_LDR (void) {
	unsigned long Offset, Address;
	long long Value;

	Address = state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	Offset  = Address & 7;

	if (!LD_VAddr((Address & ~7),&Value)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("LDL TLB: %X",Address);
#endif
		return;
	}

	state->GPR[Opcode.rt].DW = state->GPR[Opcode.rt].DW & LDR_MASK[Offset];
	state->GPR[Opcode.rt].DW += Value >> LDR_SHIFT[Offset];

}

void  r4300i_LB (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if (Opcode.rt == 0) { return; }
	if (!LB_VAddr(Address,&state->GPR[Opcode.rt].UB[0])) {
		if (ShowTLBMisses) {
#ifndef EXTERNAL_RELEASE
			printf("Error: LB TLB: %X\n",Address);
#endif
		}
		TLB_READ_EXCEPTION(Address);
	} else {
		state->GPR[Opcode.rt].DW = state->GPR[Opcode.rt].B[0];
	}
}

void  r4300i_LH (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if ((Address & 1) != 0) { ADDRESS_ERROR_EXCEPTION(Address,1); }
	if (!LH_VAddr(Address,&state->GPR[Opcode.rt].UHW[0])) {
		if (ShowTLBMisses) {
			printf("Error: LH TLB: %X at %08x, %08x\n",Address, state->PC, state->GPR[0x10].UW[0]);
		}
		TLB_READ_EXCEPTION(Address);
	} else {
		state->GPR[Opcode.rt].DW = state->GPR[Opcode.rt].HW[0];
	}
}

unsigned long LWL_MASK[4] = { 0,0xFF,0xFFFF,0xFFFFFF };
int LWL_SHIFT[4] = { 0, 8, 16, 24};

void  r4300i_LWL (void) {
	unsigned long Offset, Address, Value;

	Address = state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	Offset  = Address & 3;

	if (!LW_VAddr((Address & ~3),&Value)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("LDL TLB: %X",Address);
#endif
		return;
	}

	state->GPR[Opcode.rt].DW = (int)(state->GPR[Opcode.rt].W[0] & LWL_MASK[Offset]);
	state->GPR[Opcode.rt].DW += (int)(Value << LWL_SHIFT[Offset]);
}

void  r4300i_LW (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if ((Address & 3) != 0) { ADDRESS_ERROR_EXCEPTION(Address,1); }

#if (!defined(EXTERNAL_RELEASE))
	Log_LW(PROGRAM_COUNTER,Address);
#endif
	if (Opcode.rt == 0) { return; }

	if (!LW_VAddr(Address,&state->GPR[Opcode.rt].UW[0])) {
		if (ShowTLBMisses) {
			printf("Error: LW TLB: %X from %08x\n",Address, PROGRAM_COUNTER);
		}
		TLB_READ_EXCEPTION(Address);
	} else {
		state->GPR[Opcode.rt].DW = state->GPR[Opcode.rt].W[0];
		//TranslateVaddr(&Address);
		//if (Address == 0x00090AA0) {
		//	LogMessage("%X: Read %X from %X",PROGRAM_COUNTER,state->GPR[Opcode.rt].UW[0],state->GPR[Opcode.base].UW[0] + (short)Opcode.offset);
		//}
	}
}

void  r4300i_LBU (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if (!LB_VAddr(Address,&state->GPR[Opcode.rt].UB[0])) {
		if (ShowTLBMisses) {
			printf("Error: LBU TLB: %X\n",Address);
		}
		TLB_READ_EXCEPTION(Address);
	} else {
		state->GPR[Opcode.rt].UDW = state->GPR[Opcode.rt].UB[0];
	}
}

void  r4300i_LHU (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if ((Address & 1) != 0) { ADDRESS_ERROR_EXCEPTION(Address,1); }
	if (!LH_VAddr(Address,&state->GPR[Opcode.rt].UHW[0])) {
		if (ShowTLBMisses) {
			printf("Error: LHU TLB: %X\n",Address);
		}
		TLB_READ_EXCEPTION(Address);
	} else {
		state->GPR[Opcode.rt].UDW = state->GPR[Opcode.rt].UHW[0];
	}
}

unsigned long LWR_MASK[4] = { 0xFFFFFF00, 0xFFFF0000, 0xFF000000, 0 };
int LWR_SHIFT[4] = { 24, 16 ,8, 0 };

void  r4300i_LWR (void) {
	unsigned long Offset, Address, Value;

	Address = state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	Offset  = Address & 3;

	if (!LW_VAddr((Address & ~3),&Value)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("LDL TLB: %X",Address);
#endif
		return;
	}

	state->GPR[Opcode.rt].DW = (int)(state->GPR[Opcode.rt].W[0] & LWR_MASK[Offset]);
	state->GPR[Opcode.rt].DW += (int)(Value >> LWR_SHIFT[Offset]);
}

void  r4300i_LWU (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if ((Address & 3) != 0) { ADDRESS_ERROR_EXCEPTION(Address,1); }
	if (Opcode.rt == 0) { return; }

	if (!LW_VAddr(Address,&state->GPR[Opcode.rt].UW[0])) {
		if (ShowTLBMisses) {
			printf("Error: LWU TLB: %X\n",Address);
		}
		TLB_READ_EXCEPTION(Address);
	} else {
		state->GPR[Opcode.rt].UDW = state->GPR[Opcode.rt].UW[0];
	}
}

void  r4300i_SB (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if (!SB_VAddr(Address,state->GPR[Opcode.rt].UB[0])) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SB TLB: %X",Address);
#endif
	}
}

void  r4300i_SH (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if ((Address & 1) != 0) { ADDRESS_ERROR_EXCEPTION(Address,0); }
	if (!SH_VAddr(Address,state->GPR[Opcode.rt].UHW[0])) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SH TLB: %X",Address);
#endif
	}
}

unsigned long SWL_MASK[4] = { 0,0xFF000000,0xFFFF0000,0xFFFFFF00 };
int SWL_SHIFT[4] = { 0, 8, 16, 24 };

void  r4300i_SWL (void) {
	unsigned long Offset, Address, Value;

	Address = state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	Offset  = Address & 3;

	if (!LW_VAddr((Address & ~3),&Value)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SWL TLB: %X",Address);
#endif
		return;
	}

	Value &= SWL_MASK[Offset];
	Value += state->GPR[Opcode.rt].UW[0] >> SWL_SHIFT[Offset];

	if (!SW_VAddr((Address & ~0x03),Value)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SWL TLB: %X",Address);
#endif
	}
}


void  r4300i_SW (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if ((Address & 3) != 0) { ADDRESS_ERROR_EXCEPTION(Address,0); }
#if (!defined(EXTERNAL_RELEASE))
	Log_SW(PROGRAM_COUNTER,Address,state->GPR[Opcode.rt].UW[0]);
#endif
	if (!SW_VAddr(Address,state->GPR[Opcode.rt].UW[0])) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SW TLB: %X",Address);
#endif
	}
	//TranslateVaddr(&Address);
	//if (Address == 0x00090AA0) {
	//	LogMessage("%X: Write %X to %X",PROGRAM_COUNTER,state->GPR[Opcode.rt].UW[0],state->GPR[Opcode.base].UW[0] + (short)Opcode.offset);
	//}
}

long long SDL_MASK[8] = { 0,0xFF00000000000000ULL,
						0xFFFF000000000000ULL,
						0xFFFFFF0000000000ULL,
						0xFFFFFFFF00000000ULL,
					    0xFFFFFFFFFF000000ULL,
						0xFFFFFFFFFFFF0000ULL,
						0xFFFFFFFFFFFFFF00ULL
					};
int SDL_SHIFT[8] = { 0, 8, 16, 24, 32, 40, 48, 56 };

void  r4300i_SDL (void) {
	unsigned long Offset, Address;
	long long Value;

	Address = state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	Offset  = Address & 7;

	if (!LD_VAddr((Address & ~7),&Value)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SDL TLB: %X",Address);
#endif
		return;
	}

	Value &= SDL_MASK[Offset];
	Value += state->GPR[Opcode.rt].UDW >> SDL_SHIFT[Offset];

	if (!SD_VAddr((Address & ~7),Value)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SDL TLB: %X",Address);
#endif
	}
}

long long SDR_MASK[8] = { 0x00FFFFFFFFFFFFFFULL,
					  0x0000FFFFFFFFFFFFULL,
					  0x000000FFFFFFFFFFULL,
					  0x00000000FFFFFFFFULL,
					  0x0000000000FFFFFFULL,
					  0x000000000000FFFFULL,
					  0x00000000000000FFULL,
					  0x0000000000000000ULL
					};
int SDR_SHIFT[8] = { 56,48,40,32,24,16,8,0 };

void  r4300i_SDR (void) {
	unsigned long Offset, Address;
	long long Value;

	Address = state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	Offset  = Address & 7;

	if (!LD_VAddr((Address & ~7),&Value)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SDL TLB: %X",Address);
#endif
		return;
	}

	Value &= SDR_MASK[Offset];
	Value += state->GPR[Opcode.rt].UDW << SDR_SHIFT[Offset];

	if (!SD_VAddr((Address & ~7),Value)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SDL TLB: %X",Address);
#endif
	}
}

unsigned long SWR_MASK[4] = { 0x00FFFFFF,0x0000FFFF,0x000000FF,0x00000000 };
int SWR_SHIFT[4] = { 24, 16 , 8, 0  };

void  r4300i_SWR (void) {
	unsigned long Offset, Address, Value;

	Address = state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	Offset  = Address & 3;

	if (!LW_VAddr((Address & ~3),&Value)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SWL TLB: %X",Address);
#endif
		return;
	}

	Value &= SWR_MASK[Offset];
	Value += state->GPR[Opcode.rt].UW[0] << SWR_SHIFT[Offset];

	if (!SW_VAddr((Address & ~0x03),Value)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SWL TLB: %X",Address);
#endif
	}
}

void  r4300i_CACHE (void) {
#if (!defined(EXTERNAL_RELEASE))
	//if (!LogOptions.LogCache) { return; }
	//LogMessage("%08X: Cache operation %d, 0x%08X", PROGRAM_COUNTER, Opcode.rt,
	//	state->GPR[Opcode.base].UW[0] + (short)Opcode.offset );
#endif
}

void  r4300i_LL (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if ((Address & 3) != 0) { ADDRESS_ERROR_EXCEPTION(Address,1); }

	if (Opcode.rt == 0) { return; }

	if (!LW_VAddr(Address,&state->GPR[Opcode.rt].UW[0])) {
		if (ShowTLBMisses) {
			printf("Error: LW TLB: %X\n",Address);
		}
		TLB_READ_EXCEPTION(Address);
	} else {
		state->GPR[Opcode.rt].DW = state->GPR[Opcode.rt].W[0];
	}
	LLBit = 1;
	LLAddr = Address;
	//TranslateVaddr(&LLAddr);
	LLAddr = TLB_Map[LLAddr >> TLB_GRAN] + LLAddr;
	printf("This one at %08x\n", state->PC);
}

void  r4300i_LWC1 (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (unsigned long)((short)Opcode.offset);
	TEST_COP1_USABLE_EXCEPTION
	if ((Address & 3) != 0) { ADDRESS_ERROR_EXCEPTION(Address,1); }
	if (!LW_VAddr(Address,&*(unsigned long *)FPRFloatLocation[Opcode.ft])) {
		if (ShowTLBMisses) {
			printf("Error: LWC1 TLB: %X at %08x\n",Address, state->PC);
		}
		TLB_READ_EXCEPTION(Address);
	}
}

void  r4300i_SC (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if ((Address & 3) != 0) { ADDRESS_ERROR_EXCEPTION(Address,0); }
#if (!defined(EXTERNAL_RELEASE))
	Log_SW(PROGRAM_COUNTER,Address,state->GPR[Opcode.rt].UW[0]);
#endif
	if (LLBit == 1) {
		if (!SW_VAddr(Address,state->GPR[Opcode.rt].UW[0])) {
			printf("Error: SW TLB: %X\n",Address);
		}
	}
	state->GPR[Opcode.rt].UW[0] = LLBit;
}

void  r4300i_LD (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if ((Address & 7) != 0) { ADDRESS_ERROR_EXCEPTION(Address,1); }
	if (!LD_VAddr(Address, &(state->GPR[Opcode.rt].UDW) )) {
#ifndef EXTERNAL_RELEASE
		DisplayError("LD TLB: %X",Address);
#endif
	}
#ifdef Interpreter_StackTest
	if (Opcode.rt == 29) {
		StackValue = state->GPR[Opcode.rt].W[0];
	}
#endif
}


void  r4300i_LDC1 (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;

	TEST_COP1_USABLE_EXCEPTION
	if ((Address & 7) != 0) { ADDRESS_ERROR_EXCEPTION(Address,1); }
	if (!LD_VAddr(Address,&*(unsigned long long *)FPRDoubleLocation[Opcode.ft])) {
#ifndef EXTERNAL_RELEASE
		DisplayError("LD TLB: %X",Address);
#endif
	}
}

void  r4300i_SWC1 (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	TEST_COP1_USABLE_EXCEPTION
	if ((Address & 3) != 0) { ADDRESS_ERROR_EXCEPTION(Address,0); }

	if (!SW_VAddr(Address,*(unsigned long *)FPRFloatLocation[Opcode.ft])) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SWC1 TLB: %X",Address);
#endif
	}
}

void  r4300i_SDC1 (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;

	TEST_COP1_USABLE_EXCEPTION
	if ((Address & 7) != 0) { ADDRESS_ERROR_EXCEPTION(Address,0); }
	if (!SD_VAddr(Address,*(long long *)FPRDoubleLocation[Opcode.ft])) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SDC1 TLB: %X",Address);
#endif
	}
}

void  r4300i_SD (void) {
	unsigned long Address =  state->GPR[Opcode.base].UW[0] + (short)Opcode.offset;
	if ((Address & 7) != 0) { ADDRESS_ERROR_EXCEPTION(Address,0); }
	if (!SD_VAddr(Address,state->GPR[Opcode.rt].UDW)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("SD TLB: %X",Address);
#endif
	}
}
/********************** R4300i OpCodes: Special **********************/
void  r4300i_SPECIAL_SLL (void) {
	state->GPR[Opcode.rd].DW = (state->GPR[Opcode.rt].W[0] << Opcode.sa);
}

void  r4300i_SPECIAL_SRL (void) {
	state->GPR[Opcode.rd].DW = (int)(state->GPR[Opcode.rt].UW[0] >> Opcode.sa);
}

void  r4300i_SPECIAL_SRA (void) {
	state->GPR[Opcode.rd].DW = (state->GPR[Opcode.rt].W[0] >> Opcode.sa);
}

void  r4300i_SPECIAL_SLLV (void) {
	if (Opcode.rd == 0) { return; }
	state->GPR[Opcode.rd].DW = (state->GPR[Opcode.rt].W[0] << (state->GPR[Opcode.rs].UW[0] & 0x1F));
}

void  r4300i_SPECIAL_SRLV (void) {
	state->GPR[Opcode.rd].DW = (int)(state->GPR[Opcode.rt].UW[0] >> (state->GPR[Opcode.rs].UW[0] & 0x1F));
}

void  r4300i_SPECIAL_SRAV (void) {
	state->GPR[Opcode.rd].DW = (state->GPR[Opcode.rt].W[0] >> (state->GPR[Opcode.rs].UW[0] & 0x1F));
}

void  r4300i_SPECIAL_JR (void) {
	NextInstruction = DELAY_SLOT;
	JumpToLocation = state->GPR[Opcode.rs].UW[0];
}

void  r4300i_SPECIAL_JALR (void) {
	NextInstruction = DELAY_SLOT;
	JumpToLocation = state->GPR[Opcode.rs].UW[0];
	state->GPR[Opcode.rd].DW = (long)(PROGRAM_COUNTER + 8);
}

void  r4300i_SPECIAL_SYSCALL (void) {
	DoSysCallException(NextInstruction == JUMP);
	NextInstruction = JUMP;
	JumpToLocation = PROGRAM_COUNTER;
}

void  r4300i_SPECIAL_BREAK (void) {
	//DoBreakException(NextInstruction == JUMP);
	//NextInstruction = JUMP;
	//JumpToLocation = PROGRAM_COUNTER;
	WaitMode = 1;
}

void  r4300i_SPECIAL_SYNC (void) {
}

void  r4300i_SPECIAL_MFHI (void) {
	state->GPR[Opcode.rd].DW = state->HI.DW;
}

void  r4300i_SPECIAL_MTHI (void) {
	state->HI.DW = state->GPR[Opcode.rs].DW;
}

void  r4300i_SPECIAL_MFLO (void) {
	state->GPR[Opcode.rd].DW = state->LO.DW;
}

void  r4300i_SPECIAL_MTLO (void) {
	state->LO.DW = state->GPR[Opcode.rs].DW;
}

void  r4300i_SPECIAL_DSLLV (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rt].DW << (state->GPR[Opcode.rs].UW[0] & 0x3F);
}

void  r4300i_SPECIAL_DSRLV (void) {
	state->GPR[Opcode.rd].UDW = state->GPR[Opcode.rt].UDW >> (state->GPR[Opcode.rs].UW[0] & 0x3F);
}

void  r4300i_SPECIAL_DSRAV (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rt].DW >> (state->GPR[Opcode.rs].UW[0] & 0x3F);
}

void  r4300i_SPECIAL_MULT (void) {
	state->HI.DW = (long long)(state->GPR[Opcode.rs].W[0]) * (long long)(state->GPR[Opcode.rt].W[0]);
	state->LO.DW = state->HI.W[0];
	state->HI.DW = state->HI.W[1];
}

void  r4300i_SPECIAL_MULTU (void) {
	state->HI.DW = (unsigned long long)(state->GPR[Opcode.rs].UW[0]) * (unsigned long long)(state->GPR[Opcode.rt].UW[0]);
	state->LO.DW = state->HI.W[0];
	state->HI.DW = state->HI.W[1];
}

void  r4300i_SPECIAL_DIV (void) {
	if ( state->GPR[Opcode.rt].UDW != 0 ) {
		state->LO.DW = state->GPR[Opcode.rs].W[0] / state->GPR[Opcode.rt].W[0];
		state->HI.DW = state->GPR[Opcode.rs].W[0] % state->GPR[Opcode.rt].W[0];
	} else {
#ifndef EXTERNAL_RELEASE
		DisplayError("DIV by 0 ???");
#endif
	}
}

void  r4300i_SPECIAL_DIVU (void) {
	if ( state->GPR[Opcode.rt].UDW != 0 ) {
		state->LO.DW = (int)(state->GPR[Opcode.rs].UW[0] / state->GPR[Opcode.rt].UW[0]);
		state->HI.DW = (int)(state->GPR[Opcode.rs].UW[0] % state->GPR[Opcode.rt].UW[0]);
	} else {
#ifndef EXTERNAL_RELEASE
		DisplayError("DIVU by 0 ???");
#endif
	}
}

void  r4300i_SPECIAL_DMULT (void) {
	REGISTER Tmp[3];

	state->LO.UDW = (long long)state->GPR[Opcode.rs].UW[0] * (long long)state->GPR[Opcode.rt].UW[0];
	Tmp[0].UDW = (long long)state->GPR[Opcode.rs].W[1] * (long long)(long long)state->GPR[Opcode.rt].UW[0];
	Tmp[1].UDW = (long long)(long long)state->GPR[Opcode.rs].UW[0] * (long long)state->GPR[Opcode.rt].W[1];
	state->HI.UDW = (long long)state->GPR[Opcode.rs].W[1] * (long long)state->GPR[Opcode.rt].W[1];

	Tmp[2].UDW = (long long)state->LO.UW[1] + (long long)Tmp[0].UW[0] + (long long)Tmp[1].UW[0];
	state->LO.UDW += ((long long)Tmp[0].UW[0] + (long long)Tmp[1].UW[0]) << 32;
	state->HI.UDW += (long long)Tmp[0].W[1] + (long long)Tmp[1].W[1] + Tmp[2].UW[1];
}

void  r4300i_SPECIAL_DMULTU (void) {
	REGISTER Tmp[3];

	state->LO.UDW = (long long)state->GPR[Opcode.rs].UW[0] * (long long)state->GPR[Opcode.rt].UW[0];
	Tmp[0].UDW = (long long)state->GPR[Opcode.rs].UW[1] * (long long)state->GPR[Opcode.rt].UW[0];
	Tmp[1].UDW = (long long)state->GPR[Opcode.rs].UW[0] * (long long)state->GPR[Opcode.rt].UW[1];
	state->HI.UDW = (long long)state->GPR[Opcode.rs].UW[1] * (long long)state->GPR[Opcode.rt].UW[1];

	Tmp[2].UDW = (long long)state->LO.UW[1] + (long long)Tmp[0].UW[0] + (long long)Tmp[1].UW[0];
	state->LO.UDW += ((long long)Tmp[0].UW[0] + (long long)Tmp[1].UW[0]) << 32;
	state->HI.UDW += (long long)Tmp[0].UW[1] + (long long)Tmp[1].UW[1] + Tmp[2].UW[1];
}

void  r4300i_SPECIAL_DDIV (void) {
	if ( state->GPR[Opcode.rt].UDW != 0 ) {
		state->LO.DW = state->GPR[Opcode.rs].DW / state->GPR[Opcode.rt].DW;
		state->HI.DW = state->GPR[Opcode.rs].DW % state->GPR[Opcode.rt].DW;
	} else {
#ifndef EXTERNAL_RELEASE
		DisplayError("DDIV by 0 ???");
#endif
	}
}

void  r4300i_SPECIAL_DDIVU (void) {
	if ( state->GPR[Opcode.rt].UDW != 0 ) {
		state->LO.UDW = state->GPR[Opcode.rs].UDW / state->GPR[Opcode.rt].UDW;
		state->HI.UDW = state->GPR[Opcode.rs].UDW % state->GPR[Opcode.rt].UDW;
	} else {
#ifndef EXTERNAL_RELEASE
		DisplayError("DDIVU by 0 ???");
#endif
	}
}

void  r4300i_SPECIAL_ADD (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rs].W[0] + state->GPR[Opcode.rt].W[0];
}

void  r4300i_SPECIAL_ADDU (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rs].W[0] + state->GPR[Opcode.rt].W[0];
}

void  r4300i_SPECIAL_SUB (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rs].W[0] - state->GPR[Opcode.rt].W[0];
}

void  r4300i_SPECIAL_SUBU (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rs].W[0] - state->GPR[Opcode.rt].W[0];
}

void  r4300i_SPECIAL_AND (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rs].DW & state->GPR[Opcode.rt].DW;
}

void  r4300i_SPECIAL_OR (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rs].DW | state->GPR[Opcode.rt].DW;
#ifdef Interpreter_StackTest
	if (Opcode.rd == 29) {
		StackValue = state->GPR[Opcode.rd].W[0];
	}
#endif
}

void  r4300i_SPECIAL_XOR (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rs].DW ^ state->GPR[Opcode.rt].DW;
}

void  r4300i_SPECIAL_NOR (void) {
	state->GPR[Opcode.rd].DW = ~(state->GPR[Opcode.rs].DW | state->GPR[Opcode.rt].DW);
}

void  r4300i_SPECIAL_SLT (void) {
	if (state->GPR[Opcode.rs].DW < state->GPR[Opcode.rt].DW) {
		state->GPR[Opcode.rd].DW = 1;
	} else {
		state->GPR[Opcode.rd].DW = 0;
	}
}

void  r4300i_SPECIAL_SLTU (void) {
	if (state->GPR[Opcode.rs].UDW < state->GPR[Opcode.rt].UDW) {
		state->GPR[Opcode.rd].DW = 1;
	} else {
		state->GPR[Opcode.rd].DW = 0;
	}
}

void  r4300i_SPECIAL_DADD (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rs].DW + state->GPR[Opcode.rt].DW;
}

void  r4300i_SPECIAL_DADDU (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rs].DW + state->GPR[Opcode.rt].DW;
}

void  r4300i_SPECIAL_DSUB (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rs].DW - state->GPR[Opcode.rt].DW;
}

void  r4300i_SPECIAL_DSUBU (void) {
	state->GPR[Opcode.rd].DW = state->GPR[Opcode.rs].DW - state->GPR[Opcode.rt].DW;
}

void  r4300i_SPECIAL_TEQ (void) {
	if (state->GPR[Opcode.rs].DW == state->GPR[Opcode.rt].DW) {
#ifndef EXTERNAL_RELEASE
		DisplayError("Should trap this ???");
#endif
	}
}

void  r4300i_SPECIAL_DSLL (void) {
	state->GPR[Opcode.rd].DW = (state->GPR[Opcode.rt].DW << Opcode.sa);
}

void  r4300i_SPECIAL_DSRL (void) {
	state->GPR[Opcode.rd].UDW = (state->GPR[Opcode.rt].UDW >> Opcode.sa);
}

void  r4300i_SPECIAL_DSRA (void) {
	state->GPR[Opcode.rd].DW = (state->GPR[Opcode.rt].DW >> Opcode.sa);
}

void  r4300i_SPECIAL_DSLL32 (void) {
	state->GPR[Opcode.rd].DW = (state->GPR[Opcode.rt].DW << (Opcode.sa + 32));
}

void  r4300i_SPECIAL_DSRL32 (void) {
   state->GPR[Opcode.rd].UDW = (state->GPR[Opcode.rt].UDW >> (Opcode.sa + 32));
}

void  r4300i_SPECIAL_DSRA32 (void) {
	state->GPR[Opcode.rd].DW = (state->GPR[Opcode.rt].DW >> (Opcode.sa + 32));
}

/********************** R4300i OpCodes: RegImm **********************/
void  r4300i_REGIMM_BLTZ (void) {
	NextInstruction = DELAY_SLOT;
	if (state->GPR[Opcode.rs].DW < 0) {
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,0);
	} else {
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_REGIMM_BGEZ (void) {
	NextInstruction = DELAY_SLOT;
	if (state->GPR[Opcode.rs].DW >= 0) {
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,0);
	} else {
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_REGIMM_BLTZL (void) {
	if (state->GPR[Opcode.rs].DW < 0) {
		NextInstruction = DELAY_SLOT;
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,0);
	} else {
		NextInstruction = JUMP;
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_REGIMM_BGEZL (void) {
	if (state->GPR[Opcode.rs].DW >= 0) {
		NextInstruction = DELAY_SLOT;
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,0);
	} else {
		NextInstruction = JUMP;
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_REGIMM_BLTZAL (void) {
	NextInstruction = DELAY_SLOT;
	if (state->GPR[Opcode.rs].DW < 0) {
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,0);
	} else {
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
	state->GPR[31].DW= (long)(PROGRAM_COUNTER + 8);
}

void  r4300i_REGIMM_BGEZAL (void) {
	NextInstruction = DELAY_SLOT;
	if (state->GPR[Opcode.rs].DW >= 0) {
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
		TestInterpreterJump(PROGRAM_COUNTER,JumpToLocation,Opcode.rs,0);
	} else {
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
	state->GPR[31].DW = (long)(PROGRAM_COUNTER + 8);
}
/************************** COP0 functions **************************/
void  r4300i_COP0_MF (void) {
#if (!defined(EXTERNAL_RELEASE))
	if (LogOptions.LogCP0reads) {
		LogMessage("%08X: R4300i Read from %s (0x%08X)", PROGRAM_COUNTER,
			Cop0_Name[Opcode.rd], state->CP0[Opcode.rd]);
	}
#endif
	state->GPR[Opcode.rt].DW = (int)state->CP0[Opcode.rd];
}

void  r4300i_COP0_MT (void) {
#if (!defined(EXTERNAL_RELEASE))
	if (LogOptions.LogCP0changes) {
		LogMessage("%08X: Writing 0x%X to %s register (Originally: 0x%08X)",PROGRAM_COUNTER,
			state->GPR[Opcode.rt].UW[0],Cop0_Name[Opcode.rd], state->CP0[Opcode.rd]);
		if (Opcode.rd == 11) { //Compare
			LogMessage("%08X: Cause register changed from %08X to %08X",PROGRAM_COUNTER,
				CAUSE_REGISTER, (CAUSE_REGISTER & ~CAUSE_IP7));
		}
	}
#endif
	switch (Opcode.rd) {
	case 0: //Index
	case 2: //EntryLo0
	case 3: //EntryLo1
	case 5: //PageMask
	case 6: //Wired
	case 10: //Entry Hi
	case 14: //EPC
	case 16: //Config
	case 18: //WatchLo
	case 19: //WatchHi
	case 28: //Tag lo
	case 29: //Tag Hi
	case 30: //ErrEPC
		state->CP0[Opcode.rd] = state->GPR[Opcode.rt].UW[0];
		break;
	case 4: //Context
		state->CP0[Opcode.rd] = state->GPR[Opcode.rt].UW[0] & 0xFF800000;
		break;
	case 9: //Count
		state->CP0[Opcode.rd]= state->GPR[Opcode.rt].UW[0];
		ChangeCompareTimer();
		break;
	case 11: //Compare
		state->CP0[Opcode.rd] = state->GPR[Opcode.rt].UW[0];
		FAKE_CAUSE_REGISTER &= ~CAUSE_IP7;
		ChangeCompareTimer();
		break;
	case 12: //Status
		if ((state->CP0[Opcode.rd] ^ state->GPR[Opcode.rt].UW[0]) != 0) {
			int i = 0;
			state->CP0[Opcode.rd] = state->GPR[Opcode.rt].UW[0];

			if ((STATUS_REGISTER & STATUS_FR) == 0) {
				for (i = 0; i < 32; i ++) {
					FPRFloatLocation[i] = (void *)(&state->FPR[i >> 1].W[i & 1]);
					FPRDoubleLocation[i] = (void *)(&state->FPR[i >> 1].DW);
				}
			} else {
				for (i = 0; i < 32; i ++) {
					FPRFloatLocation[i] = (void *)(&state->FPR[i].W[0]); // from 1.4
					FPRDoubleLocation[i] = (void *)(&state->FPR[i].DW);
				}
			}

		} else {
			state->CP0[Opcode.rd] = state->GPR[Opcode.rt].UW[0];
		}
		if ((state->CP0[Opcode.rd] & 0x18) != 0) {
#ifndef EXTERNAL_RELEASE
			DisplayError("Left kernel mode ??");
#endif
		}
		CheckInterrupts();
		break;
	case 13: //cause
		state->CP0[Opcode.rd] &= 0xFFFFCFF;
#ifndef EXTERNAL_RELEASE
		if ((state->GPR[Opcode.rt].UW[0] & 0x300) != 0 ){ DisplayError("Set IP0 or IP1"); }
#endif
		break;
	default:
		R4300i_UnknownOpcode();
	}

}

/************************** COP0 CO functions ***********************/
void  r4300i_COP0_CO_TLBR (void) {
	unsigned long index = INDEX_REGISTER & 0x1F;
	if (!UseTlb) { return; }

	PAGE_MASK_REGISTER = state->tlb[index].PageMask.Value ;
	ENTRYHI_REGISTER = (state->tlb[index].EntryHi.Value & ~state->tlb[index].PageMask.Value) ;
	ENTRYLO0_REGISTER = state->tlb[index].EntryLo0.Value;
	ENTRYLO1_REGISTER = state->tlb[index].EntryLo1.Value;
}

void  r4300i_COP0_CO_TLBWI (void) {
	if (!UseTlb) { return; }
	WriteTlb(INDEX_REGISTER & 0x1F);
}

void  r4300i_COP0_CO_TLBWR (void) {
	if (!UseTlb) { return; }
	WriteTlb(RANDOM_REGISTER & 0x1F);
}

void  r4300i_COP0_CO_TLBP (void) {
	int i = 0;
	if (!UseTlb) { return; }

	INDEX_REGISTER |= 0x80000000;

	for (i = 0; i < 32; i ++) {
		unsigned long TlbValue = state->tlb[i].EntryHi.Value & (~state->tlb[i].PageMask.Mask << 13);
		unsigned long EntryHi = ENTRYHI_REGISTER & (~state->tlb[i].PageMask.Mask << 13);

		if (TlbValue == EntryHi) {
			int Global = (state->tlb[i].EntryHi.Value & 0x100) != 0;
			int SameAsid = ((state->tlb[i].EntryHi.Value & 0xFF) == (ENTRYHI_REGISTER & 0xFF));

			if (Global || SameAsid) {
				INDEX_REGISTER = i;
				return;
			}
		}
	}

}

void  r4300i_COP0_CO_ERET (void) {
	NextInstruction = JUMP;
	if ((STATUS_REGISTER & STATUS_ERL) != 0) {
		JumpToLocation = ERROREPC_REGISTER;
		STATUS_REGISTER &= ~STATUS_ERL;
	} else {
		JumpToLocation = EPC_REGISTER;
		STATUS_REGISTER &= ~STATUS_EXL;
	}
	LLBit = 0;
	CheckInterrupts();
}

/************************** COP1 functions **************************/
void  r4300i_COP1_MF (void) {
	TEST_COP1_USABLE_EXCEPTION
	state->GPR[Opcode.rt].DW = *(int *)FPRFloatLocation[Opcode.fs];
}

void  r4300i_COP1_DMF (void) {
	TEST_COP1_USABLE_EXCEPTION
	state->GPR[Opcode.rt].DW = *(long long *)FPRDoubleLocation[Opcode.fs];
}

void  r4300i_COP1_CF (void) {
	TEST_COP1_USABLE_EXCEPTION
	if (Opcode.fs != 31 && Opcode.fs != 0) {
#ifndef EXTERNAL_RELEASE
		DisplayError("CFC1 what register are you writing to ?");
#endif
		return;
	}
	state->GPR[Opcode.rt].DW = (int)state->FPCR[Opcode.fs];
}

void  r4300i_COP1_MT (void) {
	TEST_COP1_USABLE_EXCEPTION
	*(int *)FPRFloatLocation[Opcode.fs] = state->GPR[Opcode.rt].W[0];
}

void  r4300i_COP1_DMT (void) {
	TEST_COP1_USABLE_EXCEPTION
	*(long long *)FPRDoubleLocation[Opcode.fs] = state->GPR[Opcode.rt].DW;
}

void  r4300i_COP1_CT (void) {
	TEST_COP1_USABLE_EXCEPTION
	if (Opcode.fs == 31) {
		state->FPCR[Opcode.fs] = state->GPR[Opcode.rt].W[0];
		//switch((state->FPCR[Opcode.fs] & 3)) {
		//case 0: RoundingModel = _RC_NEAR; break;
		//case 1: RoundingModel = _RC_CHOP; break;
		//case 2: RoundingModel = _RC_UP; break;
		//case 3: RoundingModel = _RC_DOWN; break;
		//}
		RoundingModel = 0;
		return;
	}
#ifndef EXTERNAL_RELEASE
	DisplayError("CTC1 what register are you writing to ?");
#endif
}

/************************* COP1: BC1 functions ***********************/
void  r4300i_COP1_BCF (void) {
	TEST_COP1_USABLE_EXCEPTION
	NextInstruction = DELAY_SLOT;
	if ((state->FPCR[31] & FPCSR_C) == 0) {
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
	} else {
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_COP1_BCT (void) {
	TEST_COP1_USABLE_EXCEPTION
	NextInstruction = DELAY_SLOT;
	if ((state->FPCR[31] & FPCSR_C) != 0) {
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
	} else {
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_COP1_BCFL (void) {
	TEST_COP1_USABLE_EXCEPTION
	if ((state->FPCR[31] & FPCSR_C) == 0) {
		NextInstruction = DELAY_SLOT;
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
	} else {
		NextInstruction = JUMP;
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}

void  r4300i_COP1_BCTL (void) {
	TEST_COP1_USABLE_EXCEPTION
	if ((state->FPCR[31] & FPCSR_C) != 0) {
		NextInstruction = DELAY_SLOT;
		JumpToLocation = PROGRAM_COUNTER + ((short)Opcode.offset << 2) + 4;
	} else {
		NextInstruction = JUMP;
		JumpToLocation = PROGRAM_COUNTER + 8;
	}
}
/************************** COP1: S functions ************************/
__inline void Float_RoundToInteger32( int * Dest, float * Source ) {
	*Dest = (int)*Source;
}

__inline void Float_RoundToInteger64( long long * Dest, float * Source ) {
	*Dest = (long long)*Source;
}

void  r4300i_COP1_S_ADD (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(float *)FPRFloatLocation[Opcode.fd] = (*(float *)FPRFloatLocation[Opcode.fs] + *(float *)FPRFloatLocation[Opcode.ft]);
}

void  r4300i_COP1_S_SUB (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(float *)FPRFloatLocation[Opcode.fd] = (*(float *)FPRFloatLocation[Opcode.fs] - *(float *)FPRFloatLocation[Opcode.ft]);
}

void  r4300i_COP1_S_MUL (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(float *)FPRFloatLocation[Opcode.fd] = (*(float *)FPRFloatLocation[Opcode.fs] * *(float *)FPRFloatLocation[Opcode.ft]);
}

void  r4300i_COP1_S_DIV (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(float *)FPRFloatLocation[Opcode.fd] = (*(float *)FPRFloatLocation[Opcode.fs] / *(float *)FPRFloatLocation[Opcode.ft]);
}

void  r4300i_COP1_S_SQRT (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(float *)FPRFloatLocation[Opcode.fd] = (float)sqrt(*(float *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_S_ABS (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(float *)FPRFloatLocation[Opcode.fd] = (float)fabs(*(float *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_S_MOV (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(float *)FPRFloatLocation[Opcode.fd] = *(float *)FPRFloatLocation[Opcode.fs];
}

void  r4300i_COP1_S_NEG (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(float *)FPRFloatLocation[Opcode.fd] = (*(float *)FPRFloatLocation[Opcode.fs] * -1.0f);
}

void  r4300i_COP1_S_TRUNC_L (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(_RC_CHOP,_MCW_RC);
	Float_RoundToInteger64(&*(long long *)FPRDoubleLocation[Opcode.fd],&*(float *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_S_CEIL_L (void) {	//added by Witten
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(_RC_UP,_MCW_RC);
	Float_RoundToInteger64(&*(long long *)FPRDoubleLocation[Opcode.fd],&*(float *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_S_FLOOR_L (void) {	//added by Witten
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(_RC_DOWN,_MCW_RC);
	Float_RoundToInteger64(&*(long long *)FPRDoubleLocation[Opcode.fd],&*(float *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_S_ROUND_W (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(_RC_NEAR,_MCW_RC);
	Float_RoundToInteger32(&*(int *)FPRFloatLocation[Opcode.fd],&*(float *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_S_TRUNC_W (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(_RC_CHOP,_MCW_RC);
	Float_RoundToInteger32(&*(int *)FPRFloatLocation[Opcode.fd],&*(float *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_S_CEIL_W (void) {	//added by Witten
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(_RC_UP,_MCW_RC);
	Float_RoundToInteger32(&*(int *)FPRFloatLocation[Opcode.fd],&*(float *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_S_FLOOR_W (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(_RC_DOWN,_MCW_RC);
	Float_RoundToInteger32(&*(int *)FPRFloatLocation[Opcode.fd],&*(float *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_S_CVT_D (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(double *)FPRDoubleLocation[Opcode.fd] = (double)(*(float *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_S_CVT_W (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	Float_RoundToInteger32(&*(int *)FPRFloatLocation[Opcode.fd],&*(float *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_S_CVT_L (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	Float_RoundToInteger64(&*(long long *)FPRDoubleLocation[Opcode.fd],&*(float *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_S_CMP (void) {
	int less, equal, unorded, condition;
	float Temp0, Temp1;

	TEST_COP1_USABLE_EXCEPTION

	Temp0 = *(float *)FPRFloatLocation[Opcode.fs];
	Temp1 = *(float *)FPRFloatLocation[Opcode.ft];

	if (_isnan(Temp0) || _isnan(Temp1)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("Nan ?");
#endif
		less = 0;
		equal = 0;
		unorded = 1;
		if ((Opcode.funct & 8) != 0) {
#ifndef EXTERNAL_RELEASE
			DisplayError("Signal InvalidOperationException\nin r4300i_COP1_S_CMP\n%X  %ff\n%X  %ff",
				Temp0,Temp0,Temp1,Temp1);
#endif
		}
	} else {
		less = Temp0 < Temp1;
		equal = Temp0 == Temp1;
		unorded = 0;
	}

	condition = ((Opcode.funct & 4) && less) | ((Opcode.funct & 2) && equal) |
		((Opcode.funct & 1) && unorded);

	if (condition) {
		state->FPCR[31] |= FPCSR_C;
	} else {
		state->FPCR[31] &= ~FPCSR_C;
	}

}

/************************** COP1: D functions ************************/
__inline void Double_RoundToInteger32( int * Dest, double * Source ) {
	*Dest = (int)*Source;
}

__inline void Double_RoundToInteger64( long long * Dest, double * Source ) {
	*Dest = (long long)*Source;
}

void  r4300i_COP1_D_ADD (void) {
	TEST_COP1_USABLE_EXCEPTION
	*(double *)FPRDoubleLocation[Opcode.fd] = *(double *)FPRDoubleLocation[Opcode.fs] + *(double *)FPRDoubleLocation[Opcode.ft];
}

void  r4300i_COP1_D_SUB (void) {
	TEST_COP1_USABLE_EXCEPTION
	*(double *)FPRDoubleLocation[Opcode.fd] = *(double *)FPRDoubleLocation[Opcode.fs] - *(double *)FPRDoubleLocation[Opcode.ft];
}

void  r4300i_COP1_D_MUL (void) {
	TEST_COP1_USABLE_EXCEPTION
	*(double *)FPRDoubleLocation[Opcode.fd] = *(double *)FPRDoubleLocation[Opcode.fs] * *(double *)FPRDoubleLocation[Opcode.ft];
}

void  r4300i_COP1_D_DIV (void) {
	TEST_COP1_USABLE_EXCEPTION
	*(double *)FPRDoubleLocation[Opcode.fd] = *(double *)FPRDoubleLocation[Opcode.fs] / *(double *)FPRDoubleLocation[Opcode.ft];
}

void  r4300i_COP1_D_SQRT (void) {
	TEST_COP1_USABLE_EXCEPTION
	*(double *)FPRDoubleLocation[Opcode.fd] = (double)sqrt(*(double *)FPRDoubleLocation[Opcode.fs]);
}

void  r4300i_COP1_D_ABS (void) {
	TEST_COP1_USABLE_EXCEPTION
	*(double *)FPRDoubleLocation[Opcode.fd] = fabs(*(double *)FPRDoubleLocation[Opcode.fs]);
}

void  r4300i_COP1_D_MOV (void) {
	TEST_COP1_USABLE_EXCEPTION
	*(long long *)FPRDoubleLocation[Opcode.fd] = *(long long *)FPRDoubleLocation[Opcode.fs];
}

void  r4300i_COP1_D_NEG (void) {
	TEST_COP1_USABLE_EXCEPTION
	*(double *)FPRDoubleLocation[Opcode.fd] = (*(double *)FPRDoubleLocation[Opcode.fs] * -1.0);
}

void  r4300i_COP1_D_TRUNC_L (void) {	//added by Witten
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RC_CHOP,_MCW_RC);
	Double_RoundToInteger64(&*(long long *)FPRFloatLocation[Opcode.fd],&*(double *)FPRDoubleLocation[Opcode.fs] );
}

void  r4300i_COP1_D_CEIL_L (void) {	//added by Witten
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RC_UP,_MCW_RC);
	Double_RoundToInteger64(&*(long long *)FPRFloatLocation[Opcode.fd],&*(double *)FPRDoubleLocation[Opcode.fs] );
}

void  r4300i_COP1_D_FLOOR_L (void) {	//added by Witten
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(_RC_DOWN,_MCW_RC);
	Double_RoundToInteger64(&*(long long *)FPRDoubleLocation[Opcode.fd],&*(double *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_D_ROUND_W (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(_RC_NEAR,_MCW_RC);
	Double_RoundToInteger32(&*(int *)FPRFloatLocation[Opcode.fd],&*(double *)FPRDoubleLocation[Opcode.fs] );
}

void  r4300i_COP1_D_TRUNC_W (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RC_CHOP,_MCW_RC);
	Double_RoundToInteger32(&*(int *)FPRFloatLocation[Opcode.fd],&*(double *)FPRDoubleLocation[Opcode.fs] );
}

void  r4300i_COP1_D_CEIL_W (void) {	//added by Witten
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RC_UP,_MCW_RC);
	Double_RoundToInteger32(&*(int *)FPRFloatLocation[Opcode.fd],&*(double *)FPRDoubleLocation[Opcode.fs] );
}

void  r4300i_COP1_D_FLOOR_W (void) {	//added by Witten
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(_RC_DOWN,_MCW_RC);
	Double_RoundToInteger32(&*(int *)FPRDoubleLocation[Opcode.fd],&*(double *)FPRFloatLocation[Opcode.fs]);
}

void  r4300i_COP1_D_CVT_S (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(float *)FPRFloatLocation[Opcode.fd] = (float)*(double *)FPRDoubleLocation[Opcode.fs];
}

void  r4300i_COP1_D_CVT_W (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	Double_RoundToInteger32(&*(int *)FPRFloatLocation[Opcode.fd],&*(double *)FPRDoubleLocation[Opcode.fs] );
}

void  r4300i_COP1_D_CVT_L (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	Double_RoundToInteger64(&*(unsigned long long *)FPRDoubleLocation[Opcode.fd],&*(double *)FPRDoubleLocation[Opcode.fs]);
}

void  r4300i_COP1_D_CMP (void) {
	int less, equal, unorded, condition;
	REGISTER Temp0, Temp1;

	TEST_COP1_USABLE_EXCEPTION

	Temp0.DW = *(long long *)FPRDoubleLocation[Opcode.fs];
	Temp1.DW = *(long long *)FPRDoubleLocation[Opcode.ft];

	if (_isnan(Temp0.D) || _isnan(Temp1.D)) {
#ifndef EXTERNAL_RELEASE
		DisplayError("Nan ?");
#endif
		less = 0;
		equal = 0;
		unorded = 1;
		if ((Opcode.funct & 8) != 0) {
#ifndef EXTERNAL_RELEASE
			DisplayError("Signal InvalidOperationException\nin r4300i_COP1_D_CMP");
#endif
		}
	} else {
		less = Temp0.D < Temp1.D;
		equal = Temp0.D == Temp1.D;
		unorded = 0;
	}

	condition = ((Opcode.funct & 4) && less) | ((Opcode.funct & 2) && equal) |
		((Opcode.funct & 1) && unorded);

	if (condition) {
		state->FPCR[31] |= FPCSR_C;
	} else {
		state->FPCR[31] &= ~FPCSR_C;
	}
}

/************************** COP1: W functions ************************/
void  r4300i_COP1_W_CVT_S (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(float *)FPRFloatLocation[Opcode.fd] = (float)*(int *)FPRFloatLocation[Opcode.fs];
}

void  r4300i_COP1_W_CVT_D (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(double *)FPRDoubleLocation[Opcode.fd] = (double)*(int *)FPRFloatLocation[Opcode.fs];
}

/************************** COP1: L functions ************************/
void  r4300i_COP1_L_CVT_S (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(float *)FPRFloatLocation[Opcode.fd] = (float)*(long long *)FPRDoubleLocation[Opcode.fs];
}

void  r4300i_COP1_L_CVT_D (void) {
	TEST_COP1_USABLE_EXCEPTION
	// _controlfp(RoundingModel,_MCW_RC);
	*(double *)FPRDoubleLocation[Opcode.fd] = (double)*(long long *)FPRDoubleLocation[Opcode.fs];
}

/************************** Other functions **************************/
void  R4300i_UnknownOpcode (void) {


	//sprintf(Message,"%s: %08X", GS(MSG_UNHANDLED_OP), PROGRAM_COUNTER);
	//strcat(Message,"Stoping Emulation !");
	printf("Error: Unhandled opcode at %08x (%08x - %0d)\nStoping Emulation!\n", PROGRAM_COUNTER, Opcode.Hex, Opcode.op);

	exit(0);
}

