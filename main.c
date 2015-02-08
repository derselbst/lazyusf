#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <libgen.h>

#include <getopt.h>
#include <linux/limits.h>

#include <signal.h>

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

const char TotalPlayTime[]="--play-time";
const char TotalFadeTime[]="--fade-time";

static const struct
{
    char wildcard[12];
    char * replacement;
} wildcards[7] =
{
    {"%game%", game},
    {"%genre%", genre},
    {"%title%", title},
    {"%artist%", artist},
    {"%copyright%", copyright},
    {"%year%", year},
    {"", NULL} // terminal
};

char filename[512];

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

void sig(int signo)//, siginfo_t * info, ucontext_t * context)
{
    static bool firstTime=true;

    switch(signo)
    {
    case SIGUSR1:
        puts("sigusr1");
        play_time = track_time;
        break;
    case SIGINT:
        if(firstTime)
        {
            firstTime=false;
            puts("\nReceived SIGINT.");
            if(fade_type==1 || fade_type==2 || fade_type==3)
            {
                puts("Fading out...");
            }
            //disable playing forever
            track_time &= 0x7FFFFFFF;
            play_time = track_time;
            break;
        }
        else
        case SIGTERM:
        track_time = 0;
        fade_time = 0;
        break;
    }

    return;
}

void InitSigHandler(void)
{
    sigset_t blockset;
    sigemptyset(&blockset);
    sigaddset(&blockset, SIGUSR1);
    sigaddset(&blockset, SIGINT);
    sigaddset(&blockset, SIGTERM);
    sigprocmask(SIG_UNBLOCK, &blockset, NULL);

    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGUSR1);
    sigaddset(&sset, SIGINT);
    sigaddset(&sset, SIGTERM);

    struct sigaction act;
//    act.sa_flags = SA_SIGINFO;
//    act.sa_sigaction = (void (*)(int, siginfo_t*, void*)) sig;
    act.sa_mask = sset;
    act.sa_handler = sig;

    if(sigaction(SIGUSR1, &act, NULL))
    {
        printf("error setting up exception handler\n");
    }
    if(sigaction(SIGTERM, &act, NULL))
    {
        printf("error setting up exception handler\n");
    }
    if(sigaction(SIGINT, &act, NULL))
    {
        printf("error setting up exception handler\n");
    }
}

void formatOutFileName(char* filename, char* format)
{
    char * dir = dirname(filename);
    size_t lendir = strlen(dir);

    memset(filename,'\0', PATH_MAX);
    strcpy(filename, dir);
    strcat(filename, "/");
    strcat(filename+lendir, format);

    unsigned short i;
    for(i = 0; i<6; i++)
    {
        // find placeholder
        char * pch = strstr (filename+lendir, wildcards[i].wildcard);
        if(pch)
        {
            size_t lenrepl = strlen(wildcards[i].replacement);
            size_t lenwildc= strlen(wildcards[i].wildcard);

            char buf[512];
            strcpy(buf, pch+lenwildc);
            strncpy(pch, wildcards[i].replacement, lenrepl);

            memset(pch+lenrepl,'\0', PATH_MAX-lenrepl);
            strcat(pch,buf);
        }
    }
}


void usage(char progName[])
{
    printf("Usage: %s [OPTIONS] filename\n",progName);
    printf("\tfilename: A USF or miniUSF file\n");
    printf("\tThe output is written to filename.au\n\n");

    printf("\tOptions:\n");
    printf("\t%s\t\t\t\t specifies output filename; you may use placeholders (e.g. \"%%game%% - %%title%%\", avialable placeholders listed below)\n",outFileNameFormatParam);
    printf("\t%s\t%s\t changes sampling rate to a more standard value, rather than the odd values that games use\n",RoundFrequ,RoundFrequ_LONG);
    printf("\t%s NUM\t%s NUM\t\t NUM specifies the fade type: 1 - Linear; 2 - Logarithmic; 3 - half of sinewave; default: no fading\n",FadeType,FadeType_LONG);
#ifdef FLAC_SUPPORT
    printf("\t \t%s\t\t\t output is written to FLAC file\n",toFLAC);
#endif // FLAC_SUPPORT
#ifdef PLAYBACK_SUPPORT
    printf("\t \t%s\t\t on-the-fly playback, you might hear some interrupts\n",playback);
#endif // PLAYBACK_SUPPORT
    printf("\t \t%s\t\t\t use high level audio emulation, will speed up emulation, at the expense of accuracy, and potential emulation bugs\n",useAudioHle);
    printf("\t \t%s\t\t play forever\n",forever);
    printf("\t \t%s SEC\t\t set playing duration to SEC seconds\n",TotalPlayTime);
    printf("\t \t%s SEC\t\t set fading duration to SEC seconds\n",TotalFadeTime);
    printf("\t \t%s\t\t double the playing length read from usf\n",doubleLen);
    printf("\t \t%s\t\t use interpreter, slows down emulation; use it if recompiler (default) fails\n\n",useInterpreterCPU);

    puts("Avialable placeholders for output filename: (each placeholder should only appear once)");

    unsigned short i;
    for(i=0; wildcards[i].replacement!=NULL; i++)
    {
        printf("\t%s\n",wildcards[i].wildcard);
    }
    puts("");
}

extern uint32_t CPU_Type;
extern int32_t RSP_Cpu;

static struct option long_options[] =
{
    /* These options set a flag. */
    {"hle",             no_argument, &use_audiohle, 1},
#ifdef FLAC_SUPPORT
    {"flac",            no_argument, &useFlac, 1}, // set useFlac to 1 if flac specified
#endif
    /* These options don’t set a flag.
       We distinguish them by their indices. */
    {"fade-type",       required_argument, NULL, 'f'},
    {"play-time",       required_argument, NULL, 0},
    {"fade-time",       required_argument, NULL, 0},

    {"round-frequency", no_argument,       NULL, 'r'},
#ifdef PLAYBACK_SUPPORT
    {"playback",        no_argument,       NULL, 'p'},
#endif
    {"interpreter",     no_argument,       NULL, 'i'},
    {"forever",         no_argument,       NULL, 'e'},
    {"double",          no_argument,       NULL, 'd'},
    {0, 0, 0, 0}
};

int main(int argc, char** argv)
{
    if(argc<2)
    {
        usage(argv[0]);
        return 1;
    }

    InitSigHandler();



    /* getopt_long stores the option index here. */
    int option_index = 0;

    char* formatStr = NULL;
    bool playForever = false, doublePlayLength = false;

    int ch;
    while((ch = getopt_long(argc, argv, "o:f:rpied", long_options, &option_index)) != -1)
    {
        switch (ch)
        {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            printf ("option %s", long_options[option_index].name);
            if (optarg)
                printf (" with arg %s", optarg);
            printf ("\n");
            break;

        case 'r':
            round_frequency=1;
            break;

        case 'p':
            playingback=1;
            break;

        case 'i':
            CPU_Type = CPU_Interpreter;
            RSP_Cpu = CPU_Interpreter;
            break;

        case 'e': // play forever
            playForever = true;
            break;

        case 'd': // double length
            doublePlayLength = true;
            break;

        case 'f':
            fade_type=atoi(optarg);
            break;

        case 'o':
            formatStr = malloc(strlen(optarg)+1);
            strcpy(formatStr, optarg);
            // format outfilename

        case '?':
            /* getopt_long already printed an error message. */
            break;

        default:
            abort();
        }
    }

    /* Instead of reporting ‘--verbose’
       and ‘--brief’ as they are encountered,
       we report the final status resulting from them. */
//    if (verbose_flag)
//        puts ("verbose flag is set");

    /* Print any remaining command line arguments (not options). */
//    if (optind < argc)
//    {
//        printf ("non-option ARGV-elements: ");
//        while (optind < argc)
//            printf ("%s ", argv[optind++]);
//        putchar ('\n');
//    }

    do
    {

        if(!realpath(argv[optind],filename))
        {
            printf("Failed to get the full path of %s. Does it exist?\n", argv[optind]);
            continue;
        }

        if(usf_init(filename))
        {

            puts("");
            printf("Game     : %s\n", game);
            printf("Title    : %s\n", title);
            printf("Artist   : %s\n", artist);
            printf("Genre    : %s\n", genre);
            printf("Copyright: %s\n", copyright);
            printf("Year     : %s\n", year);

            puts("");

            if(playForever)
            {
                track_time |= 1 << (sizeof(uint32_t)*8 -1);
                puts("Playing forever");
            }
            else
            {
                printf("Playing for %f min\n", track_time/1000/60.0);

            }

            if(doublePlayLength)
            {
                track_time *= 2;
            }

            if(formatStr)
            {
                formatOutFileName(filename, formatStr);
            }

            if(useFlac)
            {
                strcat(filename,".flac");
            }
            else
            {
                strcat(filename,".au");
            }


            puts("");
            printf("enablecompare: %d\n", enablecompare);
            printf("enableFIFOfull: %d\n\n", enableFIFOfull);


            if(!usf_play())
            {
                printf("An Error occured while play.\n");
            }
        }
        else
        {
            printf("An Error occured while init.\n");
        }


    }
    while(++optind < argc);

    return 0;
}
