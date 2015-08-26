#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <byteswap.h>

#ifdef FLAC_SUPPORT
#include <FLAC/stream_encoder.h>
#include <FLAC/metadata.h>
#endif

#ifdef PLAYBACK_SUPPORT
#include <ao/ao.h>
#endif

#include "usf.h"
#include "memory.h"
#include "main.h"
#include "cpu.h"
#include "registers.h"

// AU file header
typedef struct
{
    uint32_t    magic;
    /* magic number */

    uint32_t    hdr_size;
    /* size of this header */

    uint32_t    data_size;
    /* length of data (optional) */

    uint32_t    encoding;
    /* data encoding format */

    uint32_t    sample_rate;
    /* samples per second */

    uint32_t    channels;
    /* number of interleaved channels */
} audioFileHeader;

int fd; /* sound device file descriptor */
int8_t useFlac = 0;

#ifdef FLAC_SUPPORT
FLAC__int32 sampleBuf[16*1024]; // buffer for flac encoder

#define metadataBlocks 2
FLAC__StreamMetadata * metadata[metadataBlocks]; // metadata object for flac
FLAC__StreamMetadata_VorbisComment_Entry entry;
FLAC__StreamEncoder * flacEncoder=NULL; // pointer to flac encoder
#endif // FLAC_SUPPORT

uint32_t SampleRate = 0;

int8_t playingback = 0;
#ifdef PLAYBACK_SUPPORT
ao_device *device;
#endif // PLAYBACK_SUPPORT

void OpenSound(void)
{
    // determine samplerate
    if (round_frequency)
    {
        // 20000 < SampleRate < 22500
        if (SampleRate<22500 && SampleRate>20000)
        {
            SampleRate=22050;
        }
        // 29000 < SampleRate < 35000
        else if (SampleRate<35000&& SampleRate>29000)
        {
            SampleRate=32000;
        }
        else
        {
            // calculate a more even freq, algorithm suggested by Chris Moeller
            SampleRate = SampleRate+25;
            SampleRate -= SampleRate % 50;
        }
    }

    if (!useFlac)
    {
        fd = open(filename, O_RDWR|O_CREAT,0644);
        if (fd < 0)
        {
            char perrormsg[256];
            strcat(perrormsg, "open of ");
            strcat(perrormsg, filename);
            strcat(perrormsg, " failed");
            perror(perrormsg);
            StopEmulation();
            return;
        }

        audioFileHeader AUheader;

        AUheader.magic=0x2e736e64;
        // the au format uses big endian
        AUheader.magic=bswap_32(AUheader.magic);

        // offset where data starts
        AUheader.hdr_size=24;
        AUheader.hdr_size=bswap_32(AUheader.hdr_size);

        // size of data is unknown
        AUheader.data_size=0xffffffff;

        // encoding is 16 bit linear pcm
        AUheader.encoding=3;
        AUheader.encoding=bswap_32(AUheader.encoding);

        AUheader.sample_rate=SampleRate;
        AUheader.sample_rate=bswap_32(AUheader.sample_rate);

        // stereo
        AUheader.channels=2;
        AUheader.channels=bswap_32(AUheader.channels);

        if (write(fd,&AUheader,sizeof(AUheader))<0)
        {
            DisplayError("An Error occurred when writing the Header for .au file! Exiting...\n");
            StopEmulation();
            Release_Memory();
            exit(3);
        }
    }
#ifdef FLAC_SUPPORT
    else
    {
        /* allocate the encoder */
        if((flacEncoder = FLAC__stream_encoder_new()) == NULL)
        {
            fprintf(stderr, "ERROR: allocating Flac Encoder. Exiting...\n");
            StopEmulation();
            Release_Memory();
            exit(3);
        }
        else
        {
            FLAC__stream_encoder_set_verify(flacEncoder,false);
            FLAC__stream_encoder_set_channels(flacEncoder,2);
            FLAC__stream_encoder_set_bits_per_sample(flacEncoder,16);
            FLAC__stream_encoder_set_sample_rate(flacEncoder,SampleRate);
            FLAC__stream_encoder_set_compression_level(flacEncoder,4);


            /* set metadata */
            if(
                (metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) == NULL ||
                (metadata[1] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL ||
                /* there are many tag (vorbiscomment) functions but these are convenient for this particular use: */
                !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ALBUM", game) ||
                !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false) || /* copy=false: let metadata object take control of entry's allocated string */
                !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ALBUMARTIST", artist) ||
                !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false) ||
                !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", artist) ||
                !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false) ||
                !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "COPYRIGHT", copyright) ||
                !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false) ||
                !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "DATE", year) ||
                !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false) ||
                !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "GENRE", genre) ||
                !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false) ||
                !FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "TITLE", title) ||
                !FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, /*copy=*/false)
            )
            {
                fprintf(stderr, "ERROR: out of memory or tag error\n");
            }

            metadata[1]->length = 1234; /* set the padding length */

            FLAC__stream_encoder_set_metadata(flacEncoder, metadata, metadataBlocks);
            /* end set metadata */

            FLAC__stream_encoder_init_file(flacEncoder,filename,NULL,NULL);
        }
    }
#endif // FLAC_SUPPORT
#ifdef PLAYBACK_SUPPORT
    if(playingback)
    {
        /* -- Initialize libao -- */
        ao_initialize();

        /* -- Setup for default driver -- */
        int default_driver;
        default_driver = ao_default_driver_id();

        ao_sample_format format;
        memset(&format, 0, sizeof(format));
        format.bits = 16;
        format.channels = 2;
        format.rate = SampleRate;
        format.byte_format = AO_FMT_BIG;

        /* -- Open driver -- */
        device = ao_open_live(default_driver, &format, NULL /* no options */);
        if (device == NULL)
        {
            fprintf(stderr, "Error opening device.\n");
            StopEmulation();
            Release_Memory();
            exit(1);
        }
    }
#endif // PLAYBACK_SUPPORT

    printf("Time [min]:\n");
}

void CloseSound(void)
{
    if(!useFlac)
    {
        close(fd);
    }

#ifdef FLAC_SUPPORT
    else
    {
        FLAC__stream_encoder_finish(flacEncoder);
        FLAC__metadata_object_delete(metadata[0]);
        FLAC__metadata_object_delete(metadata[1]);
        FLAC__stream_encoder_delete(flacEncoder);
    }
#endif // FLAC_SUPPORT
#ifdef PLAYBACK_SUPPORT
    if(playingback)
    {
        /* -- Close and shutdown libao -- */
        ao_close(device);
        ao_shutdown();
    }
#endif // PLAYBACK_SUPPORT
}


void AddBuffer(unsigned char *buf, unsigned int length)
{
#ifdef FLAC_SUPPORT
    int32_t out = 0;
#endif // FLAC_SUPPORT

    double vol = 1.0;

    if(!cpu_running)
    {
        return;
    }

    // fading
    if(!(track_time >> (sizeof(uint32_t)*8 -1)) && play_time > track_time)
    {
        switch (fade_type)
        {
        case 1:
            // linear
            vol =  1.0f - (((double)play_time - (double)track_time) / (double)fade_time);
            break;
        case 2:
            // logarithmic
            vol = 1.0f - pow(0.1, (1 - (((double)play_time - (double)track_time) / (double)fade_time)) * 1);
            break;
        case 3:
            // sine
            vol =  1.0f - sin( (((double)play_time - (double)track_time) / (double)fade_time) * 3.14159265359f / 2 );
            break;
        default:
            // none
            // in this case no need to play the fade part
            cpu_running = 0;
            printf("\n");
            return;
            break;
        }
    }

    uint32_t i = 0;
    for(i = 0; i<length; i+=2)
    {
        uint8_t byte0 = buf[i];
        uint8_t byte1 = buf[i+1];

        //merge byte0 and byte1
        int16_t n = 0;
        n |= byte1 & 0xFF;
        n <<= 8;
        n |= byte0 & 0xFF;

        n *= vol; // multiplier;

        // the au format uses big endian words
        n=bswap_16(n);

        //split n into byte0 and byte1
        byte1   = (n >> 8) & 0xFF;
        byte0    = n & 0xFF;

        //save the new values
        buf[i] = byte0;
        buf[i+1] = byte1;
    }
#ifdef PLAYBACK_SUPPORT
    if(playingback)
    {
        ao_play(device, (char*)buf, length);
    }
#endif // PLAYBACK_SUPPORT
#ifdef FLAC_SUPPORT
    if(useFlac)
    {
        for(i = 0; i < length; i=i+2)
        {
            /*for bigendian samples*/        sampleBuf[out++] = (FLAC__int32)(((FLAC__int16)(FLAC__int8)buf[i] << 8) | (FLAC__int16)buf[i+1]);
// little endian        sampleBuf[out++] = (FLAC__int32)(((FLAC__int16)(FLAC__int8)buf[i+1] << 8) | (FLAC__int16)buf[i]);
        }
        FLAC__stream_encoder_process_interleaved(flacEncoder,sampleBuf,length/4);
        // length/4 because: number_of_samples_of_orig_char_array/2 = samples_of_int16_arr, but flac wants number sample per each channel --> length_of_int16_arr / 2
    }
    else
#endif // FLAC_SUPPORT
    {
        // write raw pcm to au file
        unsigned int status = write(fd, buf, length);
        if (status != length)
        {
            perror("wrote wrong number of bytes\n");
        }
    }

    play_time += (((double)(length >> 2) / (double)SampleRate) * 1000.0);
    printf("\r%f",(play_time/60000));

    if((!(track_time >> (sizeof(uint32_t)*8 -1))) && (play_time > (track_time + fade_time)))
    {
        cpu_running = 0;
        printf("\n");
    }
}

void AiLenChanged(void)
{
    int32_t length = 0;
    uint32_t address = (AI_DRAM_ADDR_REG & 0x00FFFFF8);

    length = AI_LEN_REG & 0x3FFF8;

    AddBuffer(RDRAM+address, length);

    if(length && !(AI_STATUS_REG&0x80000000))
    {
        const float VSyncTiming = 789000.0f;
        double BytesPerSecond = 48681812.0 / (AI_DACRATE_REG + 1) * 4;
        double CountsPerSecond = (double)((((double)VSyncTiming) * (double)60.0)) * 2.0;
        double CountsPerByte = (double)CountsPerSecond / (double)BytesPerSecond;
        unsigned int IntScheduled = (unsigned int)((double)AI_LEN_REG * CountsPerByte);

        ChangeTimer(AiTimer,IntScheduled);
    }

    if(enableFIFOfull)
    {
        if(AI_STATUS_REG&0x40000000)
        {
            AI_STATUS_REG|=0x80000000;
        }
    }

    AI_STATUS_REG|=0x40000000;

}

unsigned int AiReadLength(void)
{
    AI_LEN_REG = 0;
    return AI_LEN_REG;
}

void AiDacrateChanged(unsigned  int value)
{
    AI_DACRATE_REG = value;
    SampleRate = 48681812 / (AI_DACRATE_REG + 1);
}
