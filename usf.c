#include "usf.h"
#include "cpu.h"
#include "memory.h"
#include "audio.h"

#include <stdio.h>
#include <stdlib.h>

char filename[512];
int loaded;
int playing, stopping;
int tags;
char title[512];

int enablecompare, enableFIFOfull;

int LoadUSF(char *fn) {
	FILE *fil = NULL;
	unsigned long reservedsize = 0, codesize = 0, crc = 0, tagstart = 0, reservestart = 0, filesize = 0, tagsize = 0, temp = 0;
	char buffer[16], *buffer2 = NULL, *tagbuffer = NULL;

	loaded = 0;
	playing = 0;
	stopping = 0;

	printf("Loading USF: %s\n", fn);
	fil = fopen(fn, "rb");
	if(!fil) {
		printf("Could not open USF!\n");
		return 0;
	}

	fread(buffer,4 ,1 ,fil);
	if(buffer[0] != 'P' && buffer[1] != 'S' && buffer[2] != 'F' && buffer[3] != 0x21) {
		printf("Invalid header in file!\n");
		fclose(fil);
		return 0;
	}

    fread(&reservedsize, 4, 1, fil);
    fread(&codesize, 4, 1, fil);
    fread(&crc, 4, 1, fil);

    fseek(fil, 0, SEEK_END);
    filesize = ftell(fil);

    reservestart = 0x10;
    tagstart = reservestart + reservedsize;
    tagsize = filesize - tagstart;

	if(tagsize) {
		fseek(fil, tagstart, SEEK_SET);
		fread(buffer, 5, 1, fil);

		if(buffer[0] != '[' && buffer[1] != 'T' && buffer[2] != 'A' && buffer[3] != 'G' && buffer[4] != ']') {
			printf("Errornous data in tag area! %d\n", tagsize) ;
			fclose(fil);
			return 0;
		}

		buffer2 = malloc(50001);
		tagbuffer = malloc(tagsize);

    	fread(tagbuffer, tagsize, 1, fil);

		psftag_raw_getvar(tagbuffer,"_lib",buffer2,50000);

		if(strlen(buffer2)) {
			char path[512], start = 0;
			int pathlength = 0;

			if(strrchr(fn, '/')) //linux
				pathlength = strrchr(fn, '/') - fn + 1;
			else if(strrchr(fn, '\\')) //windows
				pathlength = strrchr(fn, '\\') - fn + 1;
			else //no path
				pathlength = strlen(fn);

			strncpy(path, fn, pathlength);
			path[pathlength] = 0;
			strcat(path, buffer2);

			LoadUSF(path);
		}

		psftag_raw_getvar(tagbuffer,"_enablecompare",buffer2,50000);
		if(strlen(buffer2))
			enablecompare = 1;

		psftag_raw_getvar(tagbuffer,"_enableFIFOfull",buffer2,50000);
		if(strlen(buffer2))
			enableFIFOfull = 1;

		free(buffer2);
		buffer2 = NULL;

		free(tagbuffer);
		tagbuffer = NULL;

	}

	fseek(fil, reservestart, SEEK_SET);
	fread(&temp, 4, 1, fil);

	if(temp == 0x34365253) { //there is a rom section
		int len = 0, start = 0;
		fread(&len, 4, 1, fil);

		while(len) {
			int i = 0;
			fread(&start, 4, 1, fil);

			while(len) {
				int page = start >> 16;
				int readLen = ( ((start + len) >> 16) > page) ? (((page + 1) << 16) - start) : len;

                if(ROM_Pages[page] == 0) {
                	ROM_Pages[page] = malloc(0x10000);
                	memset(ROM_Pages[page], 0, 0x10000);
                	RomBytes+=0x10000;
                }

				fread(ROM_Pages[page] + (start & 0xffff), readLen, 1, fil);

				start += readLen;
				len -= readLen;
			}

			fread(&len, 4, 1, fil);
		}

	}

	fread(&temp, 4, 1, fil);
	if(temp == 0x34365253) {
		int len = 0, start = 0;
		fread(&len, 4, 1, fil);

		while(len) {
			int i = 0;
			fread(&start, 4, 1, fil);

			while(len) {
				int page = start >> 16;
				int readLen = ( ((start + len) >> 16) > page) ? (((page + 1) << 16) - start) : len;

                if(RAM_Pages[page] == 0) {
                	RAM_Pages[page] = malloc(0x10000);
                	memset(RAM_Pages[page], 0, 0x10000);
                	RamBytes+=0x10000;
                }

				fread(RAM_Pages[page] + (start & 0xffff), readLen, 1, fil);

				start += readLen;
				len -= readLen;
			}

			fread(&len, 4, 1, fil);
		}

	}

    fclose(fil);

	return 1;
}




//int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgs, int nWinMode) {
int main(int argc, char **argv) {
	int i = 0, j = 0;

	char *filename = argv[1];//"/mnt/sda1/music/xsf/Usf/bmhero2/Tripod.miniusf";

    if(!InitMemory()) return 0;

    if(!LoadUSF(filename)) {
    	FreeMemory();
    	return 0;
    }


    printf("Allocated ROM Memory: %d kb\nAllocated RAM Memory: %d kb\nTLB Memory: %d kb\n", RomBytes >> 10, RamBytes >> 10, (TLB_SIZE * sizeof(int*)) >> 10 );
    if(!LoadCPU()) {
    	FreeMemory();
    	return 0;
    }

    init_rsp();


    StartCpu();

    FreeMemory();

	return 0;
}


