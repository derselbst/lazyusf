#ifndef _OPS_H_
#define _OPS_H_

//R4300i OpCodes
#define	R4300i_SPECIAL				 0
#define	R4300i_REGIMM				 1
#define R4300i_J					 2
#define R4300i_JAL					 3
#define R4300i_BEQ					 4
#define R4300i_BNE					 5
#define R4300i_BLEZ					 6
#define R4300i_BGTZ					 7
#define R4300i_ADDI					 8
#define R4300i_ADDIU				 9
#define R4300i_SLTI					10
#define R4300i_SLTIU				11
#define R4300i_ANDI					12
#define R4300i_ORI					13
#define R4300i_XORI					14
#define R4300i_LUI					15
#define	R4300i_CP0					16
#define	R4300i_CP1					17
#define R4300i_BEQL					20
#define R4300i_BNEL					21
#define R4300i_BLEZL				22
#define R4300i_BGTZL				23
#define R4300i_DADDI				24
#define R4300i_DADDIU				25
#define R4300i_LDL					26
#define R4300i_LDR					27
#define R4300i_LB					32
#define R4300i_LH					33
#define R4300i_LWL					34
#define R4300i_LW					35
#define R4300i_LBU					36
#define R4300i_LHU					37
#define R4300i_LWR					38
#define R4300i_LWU					39
#define R4300i_SB					40
#define R4300i_SH					41
#define R4300i_SWL					42
#define R4300i_SW					43
#define R4300i_SDL					44
#define R4300i_SDR					45
#define R4300i_SWR					46
#define R4300i_CACHE				47
#define R4300i_LL					48
#define R4300i_LWC1					49
#define R4300i_LWC2					0x32
#define R4300i_LLD					0x34
#define R4300i_LDC1					53
#define R4300i_LDC2					0x36
#define R4300i_LD					55
#define R4300i_SC					0x38
#define R4300i_SWC1					57
#define R4300i_SWC2					0x3A
#define R4300i_SCD					0x3C
#define R4300i_SDC1					61
#define R4300i_SDC2					62
#define R4300i_SD					63

/* R4300i Special opcodes */
#define R4300i_SPECIAL_SLL			 0
#define R4300i_SPECIAL_SRL			 2
#define R4300i_SPECIAL_SRA			 3
#define R4300i_SPECIAL_SLLV			 4
#define R4300i_SPECIAL_SRLV			 6
#define R4300i_SPECIAL_SRAV			 7
#define R4300i_SPECIAL_JR			 8
#define R4300i_SPECIAL_JALR			 9
#define R4300i_SPECIAL_SYSCALL		12
#define R4300i_SPECIAL_BREAK		13
#define R4300i_SPECIAL_SYNC			15
#define R4300i_SPECIAL_MFHI			16
#define R4300i_SPECIAL_MTHI			17
#define R4300i_SPECIAL_MFLO			18
#define R4300i_SPECIAL_MTLO			19
#define R4300i_SPECIAL_DSLLV		20
#define R4300i_SPECIAL_DSRLV		22
#define R4300i_SPECIAL_DSRAV		23
#define R4300i_SPECIAL_MULT			24
#define R4300i_SPECIAL_MULTU		25
#define R4300i_SPECIAL_DIV			26
#define R4300i_SPECIAL_DIVU			27
#define R4300i_SPECIAL_DMULT		28
#define R4300i_SPECIAL_DMULTU		29
#define R4300i_SPECIAL_DDIV			30
#define R4300i_SPECIAL_DDIVU		31
#define R4300i_SPECIAL_ADD			32
#define R4300i_SPECIAL_ADDU			33
#define R4300i_SPECIAL_SUB			34
#define R4300i_SPECIAL_SUBU			35
#define R4300i_SPECIAL_AND			36
#define R4300i_SPECIAL_OR			37
#define R4300i_SPECIAL_XOR			38
#define R4300i_SPECIAL_NOR			39
#define R4300i_SPECIAL_SLT			42
#define R4300i_SPECIAL_SLTU			43
#define R4300i_SPECIAL_DADD			44
#define R4300i_SPECIAL_DADDU		45
#define R4300i_SPECIAL_DSUB			46
#define R4300i_SPECIAL_DSUBU		47
#define R4300i_SPECIAL_TGE			48
#define R4300i_SPECIAL_TGEU			49
#define R4300i_SPECIAL_TLT			50
#define R4300i_SPECIAL_TLTU			51
#define R4300i_SPECIAL_TEQ			52
#define R4300i_SPECIAL_TNE			54
#define R4300i_SPECIAL_DSLL			56
#define R4300i_SPECIAL_DSRL			58
#define R4300i_SPECIAL_DSRA			59
#define R4300i_SPECIAL_DSLL32		60
#define R4300i_SPECIAL_DSRL32		62
#define R4300i_SPECIAL_DSRA32		63

/* R4300i RegImm opcodes */
#define R4300i_REGIMM_BLTZ			0
#define R4300i_REGIMM_BGEZ			1
#define R4300i_REGIMM_BLTZL			2
#define R4300i_REGIMM_BGEZL			3
#define R4300i_REGIMM_TGEI			0x08
#define R4300i_REGIMM_TGEIU			0x09
#define R4300i_REGIMM_TLTI			0x0A
#define R4300i_REGIMM_TLTIU			0x0B
#define R4300i_REGIMM_TEQI			0x0C
#define R4300i_REGIMM_TNEI			0x0E
#define R4300i_REGIMM_BLTZAL		0x10
#define R4300i_REGIMM_BGEZAL		17
#define R4300i_REGIMM_BLTZALL		0x12
#define R4300i_REGIMM_BGEZALL		0x13

/* R4300i COP0 opcodes */
#define	R4300i_COP0_MF				 0
#define	R4300i_COP0_MT				 4

/* R4300i COP0 CO opcodes */
#define R4300i_COP0_CO_TLBR			1
#define R4300i_COP0_CO_TLBWI		2
#define R4300i_COP0_CO_TLBWR		6
#define R4300i_COP0_CO_TLBP			8
#define R4300i_COP0_CO_ERET			24

/* R4300i COP1 opcodes */
#define	R4300i_COP1_MF				0
#define	R4300i_COP1_DMF				1
#define	R4300i_COP1_CF				2
#define	R4300i_COP1_MT				4
#define	R4300i_COP1_DMT				5
#define	R4300i_COP1_CT				6
#define	R4300i_COP1_BC				8
#define R4300i_COP1_S				16
#define R4300i_COP1_D				17
#define R4300i_COP1_W				20
#define R4300i_COP1_L				21

/* R4300i COP1 BC opcodes */
#define	R4300i_COP1_BC_BCF			0
#define	R4300i_COP1_BC_BCT			1
#define	R4300i_COP1_BC_BCFL			2
#define	R4300i_COP1_BC_BCTL			3

#define R4300i_COP1_FUNCT_ADD		 0
#define R4300i_COP1_FUNCT_SUB		 1
#define R4300i_COP1_FUNCT_MUL		 2
#define R4300i_COP1_FUNCT_DIV		 3
#define R4300i_COP1_FUNCT_SQRT		 4
#define R4300i_COP1_FUNCT_ABS		 5
#define R4300i_COP1_FUNCT_MOV		 6
#define R4300i_COP1_FUNCT_NEG		 7
#define R4300i_COP1_FUNCT_ROUND_L	 8
#define R4300i_COP1_FUNCT_TRUNC_L	 9
#define R4300i_COP1_FUNCT_CEIL_L	10
#define R4300i_COP1_FUNCT_FLOOR_L	11
#define R4300i_COP1_FUNCT_ROUND_W	12
#define R4300i_COP1_FUNCT_TRUNC_W	13
#define R4300i_COP1_FUNCT_CEIL_W	14
#define R4300i_COP1_FUNCT_FLOOR_W	15
#define R4300i_COP1_FUNCT_CVT_S		32
#define R4300i_COP1_FUNCT_CVT_D		33
#define R4300i_COP1_FUNCT_CVT_W		36
#define R4300i_COP1_FUNCT_CVT_L		37
#define R4300i_COP1_FUNCT_C_F		48
#define R4300i_COP1_FUNCT_C_UN		49
#define R4300i_COP1_FUNCT_C_EQ		50
#define R4300i_COP1_FUNCT_C_UEQ		51
#define R4300i_COP1_FUNCT_C_OLT		52
#define R4300i_COP1_FUNCT_C_ULT		53
#define R4300i_COP1_FUNCT_C_OLE		54
#define R4300i_COP1_FUNCT_C_ULE		55
#define R4300i_COP1_FUNCT_C_SF		56
#define R4300i_COP1_FUNCT_C_NGLE	57
#define R4300i_COP1_FUNCT_C_SEQ		58
#define R4300i_COP1_FUNCT_C_NGL		59
#define R4300i_COP1_FUNCT_C_LT		60
#define R4300i_COP1_FUNCT_C_NGE		61
#define R4300i_COP1_FUNCT_C_LE		62
#define R4300i_COP1_FUNCT_C_NGT		63

void  r4300i_J              ( void );
void  r4300i_JAL            ( void );
void  r4300i_BNE            ( void );
void  r4300i_BEQ            ( void );
void  r4300i_BLEZ           ( void );
void  r4300i_BGTZ           ( void );
void  r4300i_ADDI           ( void );
void  r4300i_ADDIU          ( void );
void  r4300i_SLTI           ( void );
void  r4300i_SLTIU          ( void );
void  r4300i_ANDI           ( void );
void  r4300i_ORI            ( void );
void  r4300i_XORI           ( void );
void  r4300i_LUI            ( void );
void  r4300i_BEQL           ( void );
void  r4300i_BNEL           ( void );
void  r4300i_BLEZL          ( void );
void  r4300i_BGTZL          ( void );
void  r4300i_DADDIU         ( void );
void  r4300i_LDL            ( void );
void  r4300i_LDR            ( void );
void  r4300i_LB             ( void );
void  r4300i_LH             ( void );
void  r4300i_LWL            ( void );
void  r4300i_LW             ( void );
void  r4300i_LBU            ( void );
void  r4300i_LHU            ( void );
void  r4300i_LWR            ( void );
void  r4300i_LWU            ( void );
void  r4300i_SB             ( void );
void  r4300i_SH             ( void );
void  r4300i_SWL            ( void );
void  r4300i_SW             ( void );
void  r4300i_SDL            ( void );
void  r4300i_SDR            ( void );
void  r4300i_SWR            ( void );
void  r4300i_CACHE          ( void );
void  r4300i_LL             ( void );
void  r4300i_LWC1           ( void );
void  r4300i_LDC1           ( void );
void  r4300i_LD             ( void );
void  r4300i_SC             ( void );
void  r4300i_SWC1           ( void );
void  r4300i_SDC1           ( void );
void  r4300i_SD             ( void );

/********************** R4300i OpCodes: Special **********************/
void  r4300i_SPECIAL_SLL    ( void );
void  r4300i_SPECIAL_SRL    ( void );
void  r4300i_SPECIAL_SRA    ( void );
void  r4300i_SPECIAL_SLLV   ( void );
void  r4300i_SPECIAL_SRLV   ( void );
void  r4300i_SPECIAL_SRAV   ( void );
void  r4300i_SPECIAL_JR     ( void );
void  r4300i_SPECIAL_JALR   ( void );
void  r4300i_SPECIAL_SYSCALL ( void );
void  r4300i_SPECIAL_BREAK   ( void );
void  r4300i_SPECIAL_SYNC    ( void );
void  r4300i_SPECIAL_MFHI    ( void );
void  r4300i_SPECIAL_MTHI    ( void );
void  r4300i_SPECIAL_MFLO   ( void );
void  r4300i_SPECIAL_MTLO   ( void );
void  r4300i_SPECIAL_DSLLV  ( void );
void  r4300i_SPECIAL_DSRLV  ( void );
void  r4300i_SPECIAL_DSRAV  ( void );
void  r4300i_SPECIAL_MULT   ( void );
void  r4300i_SPECIAL_MULTU  ( void );
void  r4300i_SPECIAL_DIV    ( void );
void  r4300i_SPECIAL_DIVU   ( void );
void  r4300i_SPECIAL_DMULT  ( void );
void  r4300i_SPECIAL_DMULTU ( void );
void  r4300i_SPECIAL_DDIV   ( void );
void  r4300i_SPECIAL_DDIVU  ( void );
void  r4300i_SPECIAL_ADD    ( void );
void  r4300i_SPECIAL_ADDU   ( void );
void  r4300i_SPECIAL_SUB    ( void );
void  r4300i_SPECIAL_SUBU   ( void );
void  r4300i_SPECIAL_AND    ( void );
void  r4300i_SPECIAL_OR     ( void );
void  r4300i_SPECIAL_XOR    ( void );
void  r4300i_SPECIAL_NOR    ( void );
void  r4300i_SPECIAL_SLT    ( void );
void  r4300i_SPECIAL_SLTU   ( void );
void  r4300i_SPECIAL_DADD   ( void );
void  r4300i_SPECIAL_DADDU  ( void );
void  r4300i_SPECIAL_DSUB   ( void );
void  r4300i_SPECIAL_DSUBU  ( void );
void  r4300i_SPECIAL_TEQ    ( void );
void  r4300i_SPECIAL_DSLL   ( void );
void  r4300i_SPECIAL_DSRL   ( void );
void  r4300i_SPECIAL_DSRA   ( void );
void  r4300i_SPECIAL_DSLL32 ( void );
void  r4300i_SPECIAL_DSRL32 ( void );
void  r4300i_SPECIAL_DSRA32 ( void );

/********************** R4300i OpCodes: RegImm **********************/
void  r4300i_REGIMM_BLTZ    ( void );
void  r4300i_REGIMM_BGEZ    ( void );
void  r4300i_REGIMM_BLTZL   ( void );
void  r4300i_REGIMM_BGEZL   ( void );
void  r4300i_REGIMM_BLTZAL  ( void );
void  r4300i_REGIMM_BGEZAL  ( void );

/************************** COP0 functions **************************/
void  r4300i_COP0_MF        ( void );
void  r4300i_COP0_MT        ( void );

/************************** COP0 CO functions ***********************/
void  r4300i_COP0_CO_TLBR   ( void );
void  r4300i_COP0_CO_TLBWI  ( void );
void  r4300i_COP0_CO_TLBWR  ( void );
void  r4300i_COP0_CO_TLBP   ( void );
void  r4300i_COP0_CO_ERET   ( void );

/************************** COP1 functions **************************/
void  r4300i_COP1_MF        ( void );
void  r4300i_COP1_DMF       ( void );
void  r4300i_COP1_CF        ( void );
void  r4300i_COP1_MT        ( void );
void  r4300i_COP1_DMT       ( void );
void  r4300i_COP1_CT        ( void );

/************************* COP1: BC1 functions ***********************/
void  r4300i_COP1_BCF       ( void );
void  r4300i_COP1_BCT       ( void );
void  r4300i_COP1_BCFL      ( void );
void  r4300i_COP1_BCTL      ( void );

/************************** COP1: S functions ************************/
void  r4300i_COP1_S_ADD     ( void );
void  r4300i_COP1_S_SUB     ( void );
void  r4300i_COP1_S_MUL     ( void );
void  r4300i_COP1_S_DIV     ( void );
void  r4300i_COP1_S_SQRT    ( void );
void  r4300i_COP1_S_ABS     ( void );
void  r4300i_COP1_S_MOV     ( void );
void  r4300i_COP1_S_NEG     ( void );
void  r4300i_COP1_S_TRUNC_L ( void );
void  r4300i_COP1_S_CEIL_L  ( void );	//added by Witten
void  r4300i_COP1_S_FLOOR_L ( void );	//added by Witten
void  r4300i_COP1_S_ROUND_W ( void );
void  r4300i_COP1_S_TRUNC_W ( void );
void  r4300i_COP1_S_CEIL_W  ( void );	//added by Witten
void  r4300i_COP1_S_FLOOR_W ( void );
void  r4300i_COP1_S_CVT_D   ( void );
void  r4300i_COP1_S_CVT_W   ( void );
void  r4300i_COP1_S_CVT_L   ( void );
void  r4300i_COP1_S_CMP     ( void );

/************************** COP1: D functions ************************/
void  r4300i_COP1_D_ADD     ( void );
void  r4300i_COP1_D_SUB     ( void );
void  r4300i_COP1_D_MUL     ( void );
void  r4300i_COP1_D_DIV     ( void );
void  r4300i_COP1_D_SQRT    ( void );
void  r4300i_COP1_D_ABS     ( void );
void  r4300i_COP1_D_MOV     ( void );
void  r4300i_COP1_D_NEG     ( void );
void  r4300i_COP1_D_TRUNC_L ( void );	//added by Witten
void  r4300i_COP1_D_CEIL_L  ( void );	//added by Witten
void  r4300i_COP1_D_FLOOR_L ( void );	//added by Witten
void  r4300i_COP1_D_ROUND_W ( void );
void  r4300i_COP1_D_TRUNC_W ( void );
void  r4300i_COP1_D_CEIL_W  ( void );	//added by Witten
void  r4300i_COP1_D_FLOOR_W ( void );	//added by Witten
void  r4300i_COP1_D_CVT_S   ( void );
void  r4300i_COP1_D_CVT_W   ( void );
void  r4300i_COP1_D_CVT_L   ( void );
void  r4300i_COP1_D_CMP     ( void );

/************************** COP1: W functions ************************/
void  r4300i_COP1_W_CVT_S   ( void );
void  r4300i_COP1_W_CVT_D   ( void );

/************************** COP1: L functions ************************/
void  r4300i_COP1_L_CVT_S   ( void );
void  r4300i_COP1_L_CVT_D   ( void );

/************************** Other functions **************************/
void   R4300i_UnknownOpcode ( void );

extern unsigned long SWL_MASK[4], SWR_MASK[4], LWL_MASK[4], LWR_MASK[4];
extern int SWL_SHIFT[4], SWR_SHIFT[4], LWL_SHIFT[4], LWR_SHIFT[4];
extern int RoundingModel;

#endif
