#include "WAVFile.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
    WAVHeader* head1,*head2;
    WAVFile* file1,*file2;
    WAVData* data1,*data2;
    file1=WAVOpen("test.wav","rb");
    file2=WAVOpen("adan.wav","wb");
    data1=WAVReader(file1,-1,-1);
    WAVWrite(file2,data1,file1->header->data_size,44100,2,16);
    WAVClose(file1);WAVClose(file2);
}
    