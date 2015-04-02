#ifndef _USF_H_
#define _USF_H_
#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdbool.h>

extern int8_t round_frequency;
extern uint8_t cpu_running, use_audiohle, fake_seek_stopping, fade_type;
extern uint32_t fade_time /*time in miliseconds*/, track_time /*time in miliseconds*/;
extern double play_time;

extern int8_t enablecompare, enableFIFOfull;

/* metadata VARs */
extern char title[100];
extern char genre[100];
extern char artist[100];
extern char copyright[100];
extern char game[100];
extern char year[25];

bool usf_init(char * fn);
bool usf_play();

#endif

