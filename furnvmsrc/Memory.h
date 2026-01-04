
#ifndef MEMORY_H
#define MEMORY_H

#ifndef MEMORY_DECL
#define MEMORY_DECL
#endif

#include <stdio.h>
#include <stdlib.h>

typedef struct Memory Memory;

#define NO_INCLUDE_FURNVM_HEADERS_FLAG
#include "../furnvm.h"
#undef NO_INCLUDE_FURNVM_HEADERS_FLAG

struct Memory
{
    Byte Data[PROGRAM_SIZE];
};

#ifndef MEMORY_C

MEMORY_DECL
void Memory_Zero(Memory *restrict Mem);

MEMORY_DECL
void Memory_FileWrite(const Memory *restrict Mem, FILE *f);

MEMORY_DECL
void Memory_FileRead(Memory *restrict Mem, FILE *f);

MEMORY_DECL
Byte Memory_ReadByte(const Memory *restrict Mem, QWord Address);

MEMORY_DECL
void Memory_WriteByte(Memory *restrict Mem, QWord Address, Byte Value);

MEMORY_DECL
Byte Memory_IncByte(Memory *restrict Mem, QWord Address);

MEMORY_DECL
QWord Memory_ReadQWord(const Memory *restrict Mem, QWord Address);

MEMORY_DECL
void Memory_WriteQWord(Memory *restrict Mem, QWord Address, QWord Value);

MEMORY_DECL
void Memory_IncQWord(Memory *restrict Mem, QWord Address);

MEMORY_DECL
void Memory_DumpData(const Memory *restrict Mem, const size_t Begin, const size_t End);

#endif // #ifndef MEMORY_C
#endif // MEMORY_H
