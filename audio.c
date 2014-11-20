#include "usf.h"
#include "audio_hle.h"
#include "memory.h"
#include "audio.h"
#include <stdlib.h>
#include <stdio.h>


int SampleRate = 0, fd = 0, firstWrite = 1, curHeader = 0;
int bufptr=0;
int AudioFirst = 0;
int first = 1;


#if !defined(__WIN32__) && !defined(_MSC_VER)

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>


void OpenSound()
{
  	fd = open("./raw", O_RDWR);
	if (fd < 0) {
		perror("open of ./raw failed");
		exit(1);
	}
}


void AddBuffer(unsigned char *buf, unsigned long length) {

	int arg = 0, status = 0;

	if(!AudioFirst) {
		AudioFirst = 1;

		SampleRate = 48681812 / (AI_DACRATE_REG + 1);

		OpenSound();
	}

	arg = 16;	   /* sample size */
	status = ioctl(fd, SOUND_PCM_WRITE_BITS, &arg);
	if (status == -1)
    	perror("SOUND_PCM_WRITE_BITS ioctl failed");
  	if (arg != 16)
		perror("unable to set sample size");

	arg = 2;  /* mono or stereo */
	status = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &arg);
	if (status == -1)
		perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
	if (arg != 2)
		perror("unable to set number of channels");

	arg = SampleRate;	   /* sampling rate */
	status = ioctl(fd, SOUND_PCM_WRITE_RATE, &arg);
	if (status == -1)
		perror("SOUND_PCM_WRITE_WRITE ioctl failed");

   status = write(fd, buf, length); /* play it back */
    if (status != length)
      perror("wrote wrong number of bytes");

}


#else
#include <windows.h>
#include <mmsystem.h>

#define BUFFERS	16

HWAVEOUT hWaveOut = NULL;
char tempbuffer[BUFFERS][8192];
WAVEHDR header[BUFFERS];

void CALLBACK _waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {

	if(uMsg == WOM_DONE) {
		waveOutUnprepareHeader(hwo, dwParam1, sizeof(WAVEHDR));
		((WAVEHDR*)dwParam1)->lpData = 0;
	}
}

void writeAudio(HWAVEOUT hWaveOut, LPSTR data, int size)
{

    if(firstWrite) {
    	curHeader = 0;
    }

    if(!firstWrite) {
		int i = 0;
		curHeader = -1;

    	while(curHeader == -1) {
    		for(i = 0; i < BUFFERS; i++) {
    			if(header[i].lpData == 0) {
    				curHeader = i;
    				break;
    			}
    		}
    		if(curHeader == -1)
    			Sleep(1);
    	}
    }

    ZeroMemory(&header[curHeader], sizeof(WAVEHDR));
    memcpy(&tempbuffer[curHeader][0], data, size);
	header[curHeader].dwBufferLength = size;
	header[curHeader].lpData = &tempbuffer[curHeader][0];

	waveOutPrepareHeader(hWaveOut, &header[curHeader], sizeof(WAVEHDR));
    waveOutWrite(hWaveOut, &header[curHeader], sizeof(WAVEHDR));
    firstWrite = 0;
}

void OpenSound()
{
	MMRESULT result = 0;
	WAVEFORMATEX wfx;

    wfx.nSamplesPerSec = SampleRate;
    wfx.wBitsPerSample = 16;
    wfx.nChannels = 2;

    wfx.cbSize = 0;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

    if(waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, _waveOutProc, 0,CALLBACK_FUNCTION)) {
        printf(stderr, "unable to open WAVE_MAPPER device\n");
		exit(0);
    }

}

void AddBuffer(unsigned char *buf, unsigned long length) {

	if(!AudioFirst) {
		AudioFirst = 1;

		SampleRate = 48681812 / (AI_DACRATE_REG + 1);

		OpenSound();
	}

	writeAudio(hWaveOut,buf, length);

}

#endif

void AiLenChanged(unsigned long length) {
	int i = 0;
	unsigned long address = (AI_DRAM_ADDR_REG & 0x00FFFFF8);
	unsigned char buffer[65536];

	length &= 0x3FFF8;

	memcpyfn64(&buffer[0],address,length);

	AddBuffer(&buffer[0], length);

	AI_STATUS_REG|=0x40000000;

	if(enableFIFOfull && length) {
		const float VSyncTiming = 789000.0f;
		double BytesPerSecond = 48681812.0 / (AI_DACRATE_REG + 1) * 4;
		double CountsPerSecond = (double)(((double)VSyncTiming) * (double)60.0);
		double CountsPerByte = (double)CountsPerSecond / (double)BytesPerSecond;
		unsigned long IntScheduled = (unsigned long)((double)AI_LEN_REG * CountsPerByte);

		ChangeTimer(AiTimer,IntScheduled);
		AI_STATUS_REG|=0x80000000;
	}

}

unsigned long AiReadLength(void) {
	return 0;
}

