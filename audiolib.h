#ifndef _CPU_HLE_AUDIOLIB_
#define _CPU_HLE_AUDIOLIB_



#include "cpu_hle.h"
#include "os.h"

// a few of these structures/type were sequestered from SGI\Nindendo's code

typedef struct ALLink_s {
    struct ALLink_s      *next;
    struct ALLink_s      *prev;
} ALLink;

typedef struct {
    ALLink      freeList;
    ALLink      allocList;
    long         eventCount;
} ALEventQueue;


typedef struct {
	short type;
	unsigned char msg[12];
} ALEvent;


typedef struct {
    ALLink      node;
    long		delta; //microtime
    ALEvent     event;
} ALEventListItem;

int alCopy(int paddr);
int alLink(int paddr);
int alUnLink(int paddr);
int alEvtqPostEvent(int paddr) ;
int alEvtqPostEvent_Alt(int paddr);
int alAudioFrame(int paddr);

// need to remove these

typedef struct {
    unsigned char *base;
    unsigned char *cur;
    long len;
    long count;
} ALHeap;

typedef struct ALPlayer_s {
    struct ALPlayer_s *next;
    void *clientData;
    void *handler;
    long callTime;
    long samplesLeft;
} ALPlayer;


#endif
