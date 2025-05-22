#include <stdio.h>
#include <stdint.h>
#ifndef WAV_READER_H
#define WAV_READER_H
#pragma pack(1)


typedef struct {
    char riff[4];
    uint32_t chunk_size;
    char wave[4];
    char fmt[4];
    uint32_t fmtlen;
    uint16_t fmtType;
    uint16_t channels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t bytes_per_sample;
    uint16_t bits_per_sample;
    char data[4];
    uint32_t data_size;
}WAVHeader;


typedef unsigned char WAVData;

typedef struct{
    FILE *file;
    WAVHeader* header;
    long current_pos;
    int offset;
    WAVData* data;
}WAVFile;

WAVFile* WAVOpen(const char* file_path,const char* mode);
void WAVClose(WAVFile* file);
WAVData* WAVReader(WAVFile* wavfile,int size,int count);
double WAVPosInSec(WAVFile *file);
double WAVSeekInSec(WAVFile *file, float seconds, int origin);
size_t WAVWrite(WAVFile* wavfile, WAVData* wavdata, size_t data_size, int samplerate, int channels, int bitspersample);
#endif
