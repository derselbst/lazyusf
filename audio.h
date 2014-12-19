#ifndef _AUDIO_H_
#define _AUDIO_H_

uint32_t AiReadLength(void);
void AiLenChanged(void);
void AiDacrateChanged(uint32_t value);
void OpenSound(void);
void CloseSound(void);

extern int8_t useFlac;
extern int8_t playingback;
extern uint32_t SampleRate;

#endif
