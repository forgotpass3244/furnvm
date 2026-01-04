
#include "File.h"
#include <stdarg.h>

void File_Write(File_t *restrict File, const Byte *restrict Data, const size_t Length)
{
    // handle overflows as a ring buffer
    if (File->Position >= PROGRAM_SIZE)
    {
        File->Position = 0;
    }

    for (size_t i = 0; i < Length; i++)
    {
        Memory_WriteByte(&File->Mem, (File->Position), Data[i]);
        if (++File->Position >= PROGRAM_SIZE)
        {
            File->Position = 0;
        }
    }
}

void File_Close(File_t *File)
{
    File->Position = 0;
}

int fmprintf(File_t *Out, const char *Fmt, ...)
{
    char Buffer[512]; // kernel stack buffer
    va_list Args;

    va_start(Args, Fmt);
    int Len = vsnprintf(Buffer, sizeof(Buffer), Fmt, Args);
    va_end(Args);

    if (Len <= 0)
        return Len;

    // clamp if vsnprintf overflowed our buffer
    if ((size_t)Len > sizeof(Buffer))
        Len = sizeof(Buffer);

    File_Write(Out, (const Byte *)Buffer, (size_t)Len);
    return Len;
}
