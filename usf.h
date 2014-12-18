#ifndef _USF_H_
#define _USF_H_
#define _CRT_SECURE_NO_WARNINGS

#include <stdint.h>
#include <stdbool.h>

extern int8_t round_frequency;
extern uint32_t cpu_running, use_audiohle, is_paused, cpu_stopped, fake_seek_stopping;
extern uint32_t is_fading, fade_type, fade_time, is_seeking, seek_backwards, track_time;
extern double seek_time, play_time, rel_volume;

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

