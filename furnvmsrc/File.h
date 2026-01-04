
#ifndef FILE_H
#define FILE_H

#include "Memory.h"

typedef struct
{
    Memory Mem;
    QWord Position;
    
} File_t;

void File_Write(
    File_t *restrict File,
    const Byte *restrict Data,
    const size_t Length
);

void File_Close(File_t *File);

int fmprintf(File_t *Out, const char *Fmt, ...);

#endif // FILE_H
