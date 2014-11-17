#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "main.h"
#include "usf.h"
#include "audio.h"

const char RoundFrequ[]="-r";
const char RoundFrequ_LONG[]="--round-frequency";

const char FadeType[]="-f";
const char FadeType_LONG[]="--fade-type";

#ifdef FLAC_SUPPORT
const char toFLAC[]="--flac";
#endif // FLAC_SUPPORT
#ifdef PLAYBACK_SUPPORT
const char playback[]="--playback";
#endif // PLAYBACK_SUPPORT
const char useAudioHle[]="--hle";

void StopEmulation(void)
{
    //asm("int $3");
    //printf("Arrivederci!\n\n");
    //Release_Memory();
    //exit(0);
    cpu_running = 0;
}

void DisplayError (char * Message, ...)
{
    char Msg[1000];
    va_list ap;

    va_start( ap, Message );
    vsprintf( Msg, Message, ap );
    va_end( ap );

    printf("Error: %s\n", Msg);
}

//void UsfSleep(int32_t time)
//{
//	usleep(time * 1000);
//}

void usage()
{
    printf(
        "\tUseage: lazyusf filename [OPTIONS]\n"
        "\tfilename: A USF or miniUSF file\n"
        "\tThe output is written to filename.au\n\n"

        "\tOptions:\n"
        "\t%s\t%s\t changes sampling rate to a more standard value, rather than the odd values that games use\n"
        "\t%s NUM\t%s NUM\t\t NUM specifies the fade type: 1 - Linear; 2 - Logarithmic; 3 - half of sinewave; default: no fading\n"
#ifdef FLAC_SUPPORT
        "\t \t%s\t\t\t output is written to FLAC file\n"
#endif // FLAC_SUPPORT
#ifdef PLAYBACK_SUPPORT
        "\t \t%s\t\t on-the-fly playback, you might hear some interrupts\n"
#endif // PLAYBACK_SUPPORT
        "\t \t%s\t\t\t use high level audio emulation, will speed up emulation, at the expense of accuracy, and potential emulation bugs. \n",
        RoundFrequ,
        RoundFrequ_LONG,
        FadeType,
        FadeType_LONG,
#ifdef FLAC_SUPPORT
        toFLAC,
#endif // FLAC_SUPPORT
#ifdef PLAYBACK_SUPPORT
        playback,
#endif
        useAudioHle);
}

extern uint32_t CPU_Type;
extern int32_t RSP_Cpu;
int main(int argc, char** argv)
{
    if(argc<2)
    {
        usage();
        return 2;
    }

    uint8_t i;
    for (i = 2; i < argc; i++)
    {
        if (((strcmp(argv[i],"-h"))==0) || ((strcmp(argv[i],"--help"))==0) || ((strcmp(argv[i],"--usage"))==0))
        {
            usage();
            return 0;
        }
        else if (((strcmp(argv[i],FadeType_LONG))==0) || ((strcmp(argv[i],FadeType))==0))
        {
            if (argv[i+1]!=NULL)
            {
                fade_type=atoi(argv[++i]);
            }
        }
        else if (((strcmp(argv[i],RoundFrequ_LONG))==0) || ((strcmp(argv[i],RoundFrequ))==0))
        {
            round_frequency=1;
        }
#ifdef FLAC_SUPPORT
        else if (((strcmp(argv[i],toFLAC))==0))
        {
            useFlac=1;
        }
#endif // FLAC_SUPPORT
#ifdef PLAYBACK_SUPPORT
        else if (((strcmp(argv[i],playback))==0))
        {
            playingback=1;
        }
#endif // PLAYBACK_SUPPORT
        else if (((strcmp(argv[i],useAudioHle))==0))
        {
            use_audiohle=1;
        }
        else
        {
            printf("Warning: Unknown commandline option %s\n", argv[i]);
        }
    }

    strcpy(filename,argv[1]);

    if(useFlac)
    {
        strcat(filename,".flac");
    }
    else
    {
        strcat(filename,".au");
    }

    if(!usf_play(argv[1]))
    {
        printf("An Error occured while playing.\n");
    }

    return 0;
}
