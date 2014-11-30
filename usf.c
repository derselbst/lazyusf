#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "main.h"
#include "audio.h"
#include "psftag.h"
#include "cpu.h"
#include "memory.h"
#include "usf.h"

char filename[512];
uint32_t cpu_running = 0;
uint32_t use_audiohle = 0;
uint32_t is_paused = 0;
uint32_t cpu_stopped = 1;
uint32_t fake_seek_stopping = 0;
uint32_t is_fading = 0;

// Fade Type: 1: linear; 2: logarithmic; 3: half of a sin wave
uint32_t fade_type = 0;
uint32_t fade_time = 10000;

// Round Frequency - if 1: changes playback frequency to a more standard value, rather than the odd values that games use
int8_t round_frequency = 0;

// default track time: 360000 ms = 360 s = 6 min
// if MSB set: play forever
uint32_t track_time = 360000;
double play_time = 0.0;

// initialise with -1, indicating that vars have not been set
int8_t enablecompare = -1;
int8_t enableFIFOfull = -1;

/* METADATA VARs */
char title[100];
char genre[100];
char artist[100];
char copyright[100];
char game[100];
char year[25];

extern int32_t RSP_Cpu;

uint32_t get_length_from_string(uint8_t * str_length)
{
    uint32_t ttime = 0, temp = 0, mult = 1, level = 1;
    char Source[1024];
    char * src = Source + strlen((char*)str_length);
    strcpy(&Source[1], (char*)str_length);
    Source[0] = 0;

    while(*src)
    {
        if((*src >= '0') && (*src <= '9'))
        {
            temp += ((*src - '0') * mult);
            mult *= 10;
        }
        else
        {
            mult = 1;
            if(*src == '.')
            {
                ttime = temp;
                temp = 0;
            }
            else if(*src  == ':')
            {
                ttime += (temp * (1000 * level));
                temp = 0;
                level *= 60;
            }
        }
        src--;
    }

    ttime += (temp * (1000 * level));
    return ttime;
}

int LoadUSF(const char * fn)
{
    FILE * fil = NULL;
    uint32_t reservedsize = 0, codesize = 0, crc = 0, tagstart = 0, reservestart = 0;
    uint32_t filesize = 0, tagsize = 0, temp = 0;
    char buffer[16], * buffer2 = NULL, * tagbuffer = NULL;

    is_fading = 0;
    play_time = 0;

    fil = fopen(fn, "rb");

    if(!fil)
    {
        printf("Could not open USF!\n");
        return 0;
    }

    fread(buffer,4 ,1 ,fil);
    if(buffer[0] != 'P' && buffer[1] != 'S' && buffer[2] != 'F' && buffer[3] != 0x21)
    {
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

    if(tagsize)
    {
        fseek(fil, tagstart, SEEK_SET);
        fread(buffer, 5, 1, fil);

        if(buffer[0] != '[' && buffer[1] != 'T' && buffer[2] != 'A' && buffer[3] != 'G' && buffer[4] != ']')
        {
            printf("Errornous data in tag area! %ld\n", (long int)tagsize);
            fclose(fil);
            return 0;
        }

        buffer2 = malloc(50001);
        tagbuffer = malloc(tagsize);

        fread(tagbuffer, tagsize, 1, fil);

        psftag_raw_getvar(tagbuffer,"_lib",buffer2,50000);

        if(strlen(buffer2))
        {
            char path[512];
            int pathlength = 0;

            if(strrchr(fn, '/')) //linux
            {
                pathlength = strrchr(fn, '/') - fn + 1;
            }
            else if(strrchr(fn, '\\')) //windows
            {
                pathlength = strrchr(fn, '\\') - fn + 1;
            }
            else //no path
            {
                pathlength = strlen(fn);
            }

            strncpy(path, fn, pathlength);
            path[pathlength] = 0;
            strcat(path, buffer2);
            // load the usflib
            LoadUSF(path);
        }

        psftag_raw_getvar(tagbuffer,"_enablecompare",buffer2,50000);
        if(enablecompare == -1) // check if this has already set
        {
            if(strlen(buffer2))
            {
                enablecompare = 1;
            }
            else
            {
                enablecompare = 0;
            }
        }

        psftag_raw_getvar(tagbuffer,"_enableFIFOfull",buffer2,50000);
        if(enableFIFOfull == -1)
        {
            if(strlen(buffer2))
            {
                enableFIFOfull = 1;
            }
            else
            {
                enableFIFOfull = 0;
            }
        }

        psftag_raw_getvar(tagbuffer, "length", buffer2, 50000);
        if(strlen(buffer2))
        {
            track_time = get_length_from_string((uint8_t*)buffer2);
        }

        psftag_raw_getvar(tagbuffer, "fade", buffer2, 50000);
        if(strlen(buffer2))
        {
            fade_time = get_length_from_string((uint8_t*)buffer2);
        }

        psftag_raw_getvar(tagbuffer, "title", buffer2, 50000);
        if(strlen(buffer2))
        {
            strcpy(title, buffer2);
        }
        else
        {
            int pathlength = 0;

            if(strrchr(fn, '/')) //linux
            {
                pathlength = strrchr(fn, '/') - fn + 1;
            }
            else if(strrchr(fn, '\\')) //windows
            {
                pathlength = strrchr(fn, '\\') - fn + 1;
            }
            else //no path
            {
                pathlength = 7;
            }

            strcpy(title, &fn[pathlength]);

        }

        psftag_raw_getvar(tagbuffer, "game", buffer2, 50000);
        if(strlen(buffer2))
        {
            strcpy(game, buffer2);
        }

        psftag_raw_getvar(tagbuffer, "copyright", buffer2, 50000);
        if(strlen(buffer2))
        {
            strcpy(copyright, buffer2);
        }

        psftag_raw_getvar(tagbuffer, "artist", buffer2, 50000);
        if(strlen(buffer2))
        {
            strcpy(artist, buffer2);
        }

        psftag_raw_getvar(tagbuffer, "genre", buffer2, 50000);
        if(strlen(buffer2))
        {
            strcpy(genre, buffer2);
        }

        psftag_raw_getvar(tagbuffer, "year", buffer2, 50000);
        if(strlen(buffer2))
        {
            strcpy(year, buffer2);
        }

        free(buffer2);
        buffer2 = NULL;

        free(tagbuffer);
        tagbuffer = NULL;

    }

    fseek(fil, reservestart, SEEK_SET);
    fread(&temp, 4, 1, fil);

    if(temp == 0x34365253)   //there is a rom section
    {
        int len = 0, start = 0;
        fread(&len, 4, 1, fil);

        while(len)
        {
            fread(&start, 4, 1, fil);

            while(len)
            {
                int page = start >> 16;
                int readLen = ( ((start + len) >> 16) > page) ? (((page + 1) << 16) - start) : len;

                if(ROMPages[page] == 0)
                {
                    ROMPages[page] = malloc(0x10000);
                    memset(ROMPages[page], 0, 0x10000);
                }

                fread(ROMPages[page] + (start & 0xffff), readLen, 1, fil);

                start += readLen;
                len -= readLen;
            }

            fread(&len, 4, 1, fil);
        }

    }



    fread(&temp, 4, 1, fil);
    if(temp == 0x34365253)
    {
        int len = 0, start = 0;
        fread(&len, 4, 1, fil);

        while(len)
        {
            fread(&start, 4, 1, fil);

            fread(savestatespace + start, len, 1, fil);

            fread(&len, 4, 1, fil);
        }
    }

    // Detect the Ramsize before the memory allocation

    if(*(uint32_t*)(savestatespace + 4) == 0x400000)
    {
        RdramSize = 0x400000;
        savestatespace = realloc(savestatespace, 0x40275c);
    }
    else if(*(uint32_t*)(savestatespace + 4) == 0x800000)
    {
        RdramSize = 0x800000;
    }

    fclose(fil);

    return 1;
}


bool usf_init(char fn[])
{
//     use_audiohle = 0;
//     CPU_Type = CPU_Recompiler;
//     RSP_Cpu = CPU_Recompiler;
    
    if(!fn)
    {
        return false;
    }

    // Defaults (which would be overriden by Tags / playing
    savestatespace = NULL;
    cpu_running = is_paused = fake_seek_stopping = 0;
    cpu_stopped = 1;
    is_fading = 0;
    play_time = 0.0;

    // Allocate main memory after usf loads (to determine ram size)

    if (PreAllocate_Memory())
    {
        if(!LoadUSF(fn))
        {
            Release_Memory();
            return false;
        }

        // in case they are still unset, e.g. both files miniusf and usflib have no tag area, set them to 0 (default)
        if(enablecompare == -1 || enableFIFOfull == -1)
        {
            enablecompare = 0;
            enableFIFOfull = 0;
        }

        return true;
    }
    
    return false;
}

void usf_destroy()
{

}

//void usf_seek(InputPlayback * context, gint time)
//{
//  usf_mseek(time * 1000);
//}


//void usf_mseek(InputPlayback * context, gulong millisecond)
//{
//  if(millisecond < play_time) {
//      is_paused = 0;
//
//      fake_seek_stopping = 1;
//      CloseCpu();
//
//      while(!cpu_stopped)
//          usleep(1);
//
//      is_seeking = 1;
//      seek_time = (double)millisecond;
//
//      fake_seek_stopping = 2;
//  } else {
//      is_seeking = 1;
//      seek_time = (double)millisecond;
//  }
//
//  context->output->flush(millisecond/1000);
//}

bool usf_play()
{
        if(Allocate_Memory() != 0 )
        {
            while(1)
            {
                is_fading = 0;
                play_time = 0;

                printf("Start Emulation\n");
                StartEmulationFromSave(savestatespace);
                if(!fake_seek_stopping)
                {
                    break;
                }
                while(fake_seek_stopping != 2)
                {
                    usleep(1);
                }
                fake_seek_stopping = 4;
            }
        }
        else
        {
            return false;
        }

        Release_Memory();

        return true;
}

//void usf_stop(InputPlayback *context)
//{
//
//  is_paused = 0;
//
//  if(!cpu_running)
//      return;
//
//  CloseCpu();
//  g_thread_join(decode_thread);
//
//  Release_Memory();
//
//  context->output->close_audio();
//
//}

unsigned int usf_is_our_file(char *pFile)
{
    const char *pExt;

    if (!pFile)
    {
        return 0;
    }

    /* get extension */
    pExt = strrchr(pFile,'.');
    if (!pExt)
    {
        return 0;
    }
    /* skip past period */
    ++pExt;

    if ((strcasecmp(pExt,"usf") == 0) ||
            (strcasecmp(pExt,"miniusf") == 0))
    {
        return 1;
    }

    return 0;
}

//void usf_pause(InputPlayback *context, gshort paused)
//{
//  is_paused = paused;//is_paused?0:1;
//}

const char *usf_exts [] =
{
    "usf",
    "miniusf",
    NULL
};

int usf_get_time()
{
    return (int)play_time;
}
