#include "platform.h"
#include <math.h>

typedef struct 
{
    void *memory;
    u32 file_size;
}FileContent;

FileContent win_read_file(char *filename);