#ifndef _CPU_HLE_OS_
#define _CPU_HLE_OS_

#include "cpu_hle.h"


#pragma pack(1)


#define OS_STATE_STOPPED	1
#define OS_STATE_RUNNABLE	2
#define OS_STATE_RUNNING	4
#define OS_STATE_WAITING	8

typedef long	OSPri;
typedef long	OSId;
typedef
	union
	{
		struct
		{
			float f_odd;
			float f_even;
		} f;
		double d;
	}
__OSfp;

typedef struct {
	unsigned long long	at, v0, v1, a0, a1, a2, a3;
	unsigned long long	t0, t1, t2, t3, t4, t5, t6, t7;
	unsigned long long	s0, s1, s2, s3, s4, s5, s6, s7;
	unsigned long long	t8, t9,         gp, sp, s8, ra;
	unsigned long long	lo, hi;
	unsigned long	sr, pc, cause, badvaddr, rcp;
	unsigned long	fpcsr;
	__OSfp	 fp0,  fp2,  fp4,  fp6,  fp8, fp10, fp12, fp14;
	__OSfp	fp16, fp18, fp20, fp22, fp24, fp26, fp28, fp30;
} __OSThreadContext;

typedef struct OSThread_s
{
	struct		OSThread_s	*next;		// run/mesg queue link
	OSPri		priority;				// run/mesg queue priority
	struct		OSThread_s	**queue;	// queue thread is on
	struct		OSThread_s	*tlnext;	// all threads queue link
#if 0
	unsigned short			state;					// OS_STATE_*
	unsigned short			flags;					// flags for rmon
#endif
	//swap these because of byteswapping
	unsigned short			flags;					// flags for rmon
	unsigned short			state;					// OS_STATE_*
	OSId		id;						// id for debugging
	int			fp;						// thread has used fp unit
	__OSThreadContext	context;		// register/interrupt mask
} OSThread;

typedef void *	OSMesg;

//
// Structure for message queue
//
typedef struct OSMesgQueue_s
{
	OSThread	*mtqueue;		// Queue to store threads blocked
								//   on empty mailboxes (receive)
	OSThread	*fullqueue;		// Queue to store threads blocked
								//   on full mailboxes (send)
	long		validCount;		// Contains number of valid message
	long		first;			// Points to first valid message
	long		msgCount;		// Contains total # of messages
	OSMesg		*msg;			// Points to message buffer array
} OSMesgQueue;


int __osRestoreInt(int n);
int __osDisableInt(int n);
int __osEnqueueThread(int n) ;

int osStartThread(int n);
int osRecvMesg(int n);
int osSetIntMask(int paddr) ;
int osVirtualToPhysical(int paddr);
int osAiSetNextBuffer(int paddr);

int saveThreadContext(int paddr);
int loadThreadContext(int paddr);
#endif
