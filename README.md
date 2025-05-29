A C Library to read and write ".WAV" Files.
Contains function to Open, Read, Write(16-bit PCM), Tell and Seek in a .wav file

Here are the core functions provided:

    WAVFile *wav_open(const char *filename, const char *mode);

    int wav_read_sample(WAVFile *file);

    void wav_write_sample(WAVFile *file, int sample);

    void wav_seek(WAVFile *file, long sample_offset);

    long wav_tell(WAVFile *file);

    void wav_close(WAVFile *file);

    
