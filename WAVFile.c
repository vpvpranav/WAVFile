#include "WAVFile.h"
#include <stdlib.h>
#include <string.h>

WAVFile* WAVOpen(const char* file_path,const char* mode)
{
    if (!file_path || !mode)
    {
        printf("Invalid parameters\n");
        return NULL;
    }

    int write_mode = strcmp(mode, "wb") == 0 || strcmp(mode, "WB") == 0;

    FILE* file = fopen(file_path, "rb");

    WAVHeader* header = (WAVHeader*)malloc(sizeof(WAVHeader));
    if (!header)
    {
        printf("Memory allocation error\n");
        if (file) fclose(file);
        return NULL;
    }

    if (write_mode) 
    {
        if (!file) 
        {
            file = fopen(file_path, mode);
            if (!file) 
            {
                printf("Failed to create file\n");
                free(header);
                return NULL;
            }

            memcpy(header->riff, "RIFF", 4);
            memcpy(header->fmt, "fmt ", 4);
            memcpy(header->data, "data", 4);
            memcpy(header->wave, "WAVE", 4);
            header->fmtlen = 16;
            header->fmtType = 1;
            header->channels = 2;
            header->sampleRate = 44100;
            header->bits_per_sample = 16;
            header->bytes_per_sample = 4;
            header->byteRate = 176400;
            header->data_size = 0;
            header->chunk_size = 36;
        } 
        else 
        {
            size_t read_count = fread(header, sizeof(WAVHeader), 1, file);
            fclose(file);
            
            if (read_count != 1) 
            {
                printf("Failed to read existing header\n");
                free(header);
                return NULL;
            }
            
            file = fopen(file_path, mode);
            if (!file) 
            {
                printf("Failed to reopen file for writing\n");
                free(header);
                return NULL;
            }
        }
    } 
    else 
    {
        if (!file) 
        {
            printf("Failed to open file for reading\n");
            free(header);
            return NULL;
        }
        
        size_t read_count = fread(header, sizeof(WAVHeader), 1, file);

        if (read_count != 1) 
        {
            printf("Failed to read WAV header\n");
            fclose(file);
            free(header);
            return NULL;
        }
        fseek(file, sizeof(WAVHeader), SEEK_SET);
    }

    if (!write_mode || (write_mode && ftell(file) > 0))
    {
        if (strncmp(header->riff, "RIFF", 4) != 0 || 
            strncmp(header->wave, "WAVE", 4) != 0 ||
            (header->fmtType != 1 && header->fmtType != 3) ||
            strncmp(header->data, "data", 4) != 0) 
        {
            printf("Unsupported file format\n");
            fclose(file);
            free(header);
            return NULL;
        }
    }
    WAVFile* wavfile=(WAVFile*)malloc(sizeof(WAVFile));
    if(!wavfile){printf("Memory Error");free(header);fclose(file);return NULL;}
    wavfile->current_pos=0;
    wavfile->offset=sizeof(WAVHeader);
    wavfile->file=file;
    wavfile->header=header;
    wavfile->data=NULL;
    return wavfile;
}

WAVData* WAVReader(WAVFile* wavfile,int size,int count)
{
    if (!wavfile || !wavfile->file || !wavfile->header)
    {
        printf("Invalid WAVFile\n");
        return NULL;
    }

    size_t total_size;
    if(size==-1 || count == -1)
    {
        total_size=wavfile->header->data_size;
    }
    else
    {
        total_size = size * count;
    }
    if(total_size>wavfile->header->data_size-wavfile->current_pos)
    {
        total_size = wavfile->header->data_size-wavfile->current_pos;
    }
    WAVData* data = (WAVData*)malloc(total_size);

    if(!data)
    {
        printf("Memory Error");
        return NULL;
    }

    if (fread(data, 1, total_size, wavfile->file) != total_size)
    {
        printf("Incomplete read\n");
        free(data);
        return NULL;
    }


    if (wavfile->data)
    {
        free(wavfile->data);
    }
    wavfile->data=data;
    wavfile->current_pos=ftell(wavfile->file)-(wavfile->offset);
    return data;
}

double WAVPosInSec(WAVFile *wavfile)
{
    if (!wavfile || !wavfile->file || !wavfile->header)
    {
        printf("Invalid WAVFile\n");
        return -1;
    }
    long pos = ftell(wavfile->file);
    if (pos == -1)
    {
        printf("Error getting position\n");
        return -1;
    }
    wavfile->current_pos = pos-sizeof(WAVHeader);
    return ((double) wavfile->current_pos / wavfile->header->byteRate);
}

double WAVSeekInSec(WAVFile *wavfile, float seconds, int origin)
{
    if (!wavfile || !wavfile->file || !wavfile->header)
    {
        printf("Invalid WAVFile\n");
        return -1;
    }
    long pos;
    if(origin==0)
    {
        pos = seconds*(wavfile->header->byteRate)+wavfile->offset;
    }
    else if(origin==1)
    {
        pos = seconds*(wavfile->header->byteRate)+ftell(wavfile->file);
    }
    else
    {
        printf("Invalid Origin");
        return -1;
    }
    if(pos < (long)wavfile->offset ||pos >= wavfile->header->data_size + wavfile->offset) 
    {
        printf("Seek position exceeds data range\n");
        return -1;
    }

    if(fseek(wavfile->file,pos,0)!=0)
    {
        printf("Couldn't Seek");
        return -1;
    }
    wavfile->current_pos=ftell(wavfile->file)-wavfile->offset;
    return (double)wavfile->current_pos/wavfile->header->byteRate;
}

size_t WAVWrite(WAVFile* wavfile, WAVData* wavdata, size_t data_size, int samplerate, int channels, int bitspersample)
{
    if (!wavfile || !wavfile->file || !wavfile->header || !wavdata) {
        printf("Invalid parameters\n");
        return 0;
    }

    if(bitspersample==-1){bitspersample=16;}
    if(channels==-1){channels=2;}
    if(samplerate==-1){samplerate=44100;} 

    WAVHeader* hdr = wavfile->header;
    hdr->fmtType = 1;
    hdr->fmtlen = 16;
    hdr->channels = channels;
    hdr->bits_per_sample = bitspersample;
    hdr->sampleRate = samplerate;
    hdr->bytes_per_sample = (hdr->channels * hdr->bits_per_sample) / 8;
    hdr->byteRate = samplerate * hdr->bytes_per_sample;
    hdr->data_size = data_size;
    hdr->chunk_size = 36 + data_size;

    if (fseek(wavfile->file, 0, SEEK_SET) != 0) {
        printf("Failed to seek to beginning\n");
        return 0;
    }

    if (fwrite(wavfile->header, sizeof(WAVHeader), 1, wavfile->file) != 1) {
        printf("Failed to write header\n");
        return 0;
    }
    
    size_t written_len = fwrite(wavdata, 1, data_size, wavfile->file);
    wavfile->current_pos = written_len;
    fflush(wavfile->file);
    return written_len;
}

void WAVClose(WAVFile* wavfile)
{
    if (wavfile) {
        if (wavfile->file) {
            fclose(wavfile->file);
        }
        if (wavfile->data) {
            free(wavfile->data);
        }
        if (wavfile->header) {
            free(wavfile->header);
        }
        free(wavfile);
    }
}
