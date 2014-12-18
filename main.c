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

const char outFileNameFormatParam[]="-o";

#ifdef FLAC_SUPPORT
const char toFLAC[]="--flac";
#endif // FLAC_SUPPORT
#ifdef PLAYBACK_SUPPORT
const char playback[]="--playback";
#endif // PLAYBACK_SUPPORT
const char useAudioHle[]="--hle";
const char useInterpreterCPU[]="--interpreter";

const char forever[]="--forever";

const char doubleLen[]="--double";

char filename[512];

int InitalizeApplication ( void )
{
    return 1;
}

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
//  usleep(time * 1000);
//}

void usage(char filename[])
{
    printf(
        "Usage: %s filename [OPTIONS]\n"
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
        "\t \t%s\t\t\t use high level audio emulation, will speed up emulation, at the expense of accuracy, and potential emulation bugs. \n"
	"\t \t%s\t\t play forever\n"
	"\t \t%s\t\t double the playing length read from usf\n"
        "\t \t%s\t\t use interpreter, slows down emulation; use it if recompiler (default) fails\n\n",
        filename,
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
        useAudioHle,
        forever,
        doubleLen,
        useInterpreterCPU);
}

extern uint32_t CPU_Type;
extern int32_t RSP_Cpu;
int main(int argc, char** argv)
{
    if(argc<2)
    {
        usage(argv[0]);
        return 2;
    }

    if(usf_init(argv[1]))
    {
      strcpy(filename, argv[1]);

	uint8_t i;
      for (i = 2; i < argc; i++)
      {
	  if (((strcmp(argv[i],"-h"))==0) || ((strcmp(argv[i],"--help"))==0) || ((strcmp(argv[i],"--usage"))==0))
	  {
	      usage(argv[0]);
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
	  else if (((strcmp(argv[i],useInterpreterCPU))==0))
	  {
	      CPU_Type = CPU_Interpreter;
	      RSP_Cpu = CPU_Interpreter;
	  }
	  else if (((strcmp(argv[i],forever))==0))
	  {
	      track_time |= 1 << (sizeof(uint32_t)*8 -1);
	  }
	  else if (((strcmp(argv[i],doubleLen))==0))
	  {
	      track_time *= 2;
	  }
      else if (((strcmp(argv[i],outFileNameFormatParam))==0))
	  {
	      //TODO: preserve path to file!!!

if(!argv[++i])
    continue;

            memset(filename,'\0', sizeof(filename));
	         strcpy(filename, argv[i+1]);

    struct asdf
    {
        char wildcard[12];
        char * replacement;
    };

    struct asdf wildcards[6]=  {
                                {"%game%", game},
                                {"%genre%", genre},
                                {"%title%", title},
                                {"%artist%", artist},
                                {"%copyright%", copyright},
                                {"%year%", year}
                                };

    unsigned short i;
    for(i = 0; i<6; i++)
    {
        // find placeholder
        char * pch = strstr (filename, wildcards[i].wildcard);
        if(pch)
        {
            size_t lenrepl = strlen(wildcards[i].replacement);
            size_t lenwildc= strlen(wildcards[i].wildcard);

                char buf[512];
            strcpy(buf, pch+lenwildc);
                strncpy(pch, wildcards[i].replacement, lenrepl);

            memset(pch+lenrepl,'\0', sizeof(filename)-lenrepl);
                strcat(pch,buf);
        }
    }

    puts(filename);

	  }
	  else
	  {
	      printf("Warning: Unknown commandline option %s\n", argv[i]);
	  }
      }

      if(useFlac)
      {
	  strcat(filename,".flac");
      }
      else
      {
	  strcat(filename,".au");
      }
        if(!usf_play())
        {
            printf("An Error occured while play.\n");
        }
    }
    else
    {
        printf("An Error occured while init.\n");
    }

    return 0;
}
