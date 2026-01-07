
#ifndef MEMORY_C
#define MEMORY_C

#include <stdio.h>
#include <stdlib.h>
#include "../furnvm.h"
#include "Memory.h"

MEMORY_DECL
void Memory_Zero(Memory *restrict Mem)
{
    for (size_t i = 0; i < sizeof(Mem->Data); i++)
    {
        Mem->Data[i] = 0;
    }
}

MEMORY_DECL
Byte Memory_ReadByte(const Memory *restrict Mem, QWord Address)
{
    return ((Address > PROGRAM_SIZE) ? 0 : (Mem->Data[Address]));
}

MEMORY_DECL
void Memory_WriteByte(Memory *restrict Mem, QWord Address, Byte Value)
{
    if (Address > PROGRAM_SIZE)
    {
        return;
    }
    
    Mem->Data[Address] = Value;
}

MEMORY_DECL
Byte Memory_IncByte(Memory *restrict Mem, QWord Address)
{
    if (Address > PROGRAM_SIZE)
    {
        return 0;
    }

    return ++Mem->Data[Address];
}

MEMORY_DECL
QWord Memory_ReadQWord(const Memory *restrict Mem, QWord Address)
{
    QWord Result = 0;
    for (size_t i = 0; i < 8; i++)
    {
        Result |= ((QWord)(Mem->Data[Address + i]) << (i * 8));
    }
    return Result;
}

MEMORY_DECL
void Memory_WriteQWord(Memory *restrict Mem, QWord Address, QWord Value)
{
    for (size_t i = 0; i < 8; i++)
    {
        Mem->Data[Address + i] = (Byte)((Value >> (i * 8)) & 0xFF);
    }
}

MEMORY_DECL
void Memory_IncQWord(Memory *restrict Mem, QWord Address)
{
    Memory_WriteQWord(Mem, Address, Memory_ReadQWord(Mem, Address) + 1);
}

MEMORY_DECL
void Memory_DecQWord(Memory *restrict Mem, QWord Address)
{
    Memory_WriteQWord(Mem, Address, Memory_ReadQWord(Mem, Address) - 1);
}

MEMORY_DECL
void Memory_DumpData(const Memory *restrict Mem, const size_t Begin, const size_t End)
{
    printf("  Data:\n");
    for (unsigned int i = Begin; i < (End + 1); i++)
    {
        printf("    [%u] = %i\n", i, Memory_ReadByte(Mem, i));
    }
}

MEMORY_DECL
void Memory_FileWrite(const Memory *restrict Mem, FILE *f)
{
    // write memory to the file
    for (size_t i = 0; i < sizeof(Mem->Data); i++)
    {
        fputc(Mem->Data[i], f);
    }
}

MEMORY_DECL
void Memory_FileRead(Memory *restrict Mem, FILE *f)
{
    unsigned char Buffer[PROGRAM_SIZE];
    size_t BytesRead;

    while ((BytesRead = fread(Buffer, 1, sizeof(Buffer), f)) > 0)
    {
        for (size_t i = 0; i < BytesRead; i++)
        {
            Memory_WriteByte(Mem, i, Buffer[i]);
        }
    }
}

#endif // MEMORY_C
