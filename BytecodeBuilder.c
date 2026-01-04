
#ifdef BCBUILD_INCLUDE
#error include BytecodeBuilder.h only instead of BytecodeBuilder.c directly, then link with it after compiling
#else

#include "BytecodeBuilder.h"
#include "furnvmsrc/Memory.c"
#include <string.h>

void BCBuild_Put(BytecodeBuilder *Builder, Byte Value)
{
    Memory_WriteByte(Builder->Mem, Builder->Position++, Value);
}

void BCBuild_PutQWord(BytecodeBuilder *Builder, QWord Value)
{
    Memory_WriteQWord(Builder->Mem, Builder->Position, Value);
    Builder->Position += sizeof(QWord);
}

void BCBuild_PutAddress(BytecodeBuilder *Builder, QWord Value)
{
    BCBuild_PutQWord(Builder, Value);
}

void BCBuild_EndInstructions(BytecodeBuilder *Builder)
{
    Builder->InstructionsEnd = (Builder->Position + 1);
    Memory_WriteQWord(Builder->Mem, Builder->EndInstPtr, Builder->InstructionsEnd);
    Memory_WriteQWord(Builder->Mem, Builder->StackPointerPos, Builder->InstructionsEnd);
}

void BCBuild_EmitFile(const BytecodeBuilder *Builder, FILE *f)
{
    for (size_t i = 0; i < (PROGRAM_SIZE); i++)
    {
        fputc(Builder->Mem->Data[i], f);
    }
}

void BCBuild_Header(BytecodeBuilder *Builder)
{
    // header
    BCBuild_PutQWord(Builder, 8);   // instruction pointer
    BCBuild_Put(Builder, 'f'); // magic
    BCBuild_Put(Builder, 'u'); // magic
    BCBuild_Put(Builder, 'r'); // magic
    BCBuild_Put(Builder, 'n'); // magic

    BCBuild_PutQWord(Builder, 33); // register alloc size

    for (size_t i = 0; i < 33; i++)
    {
        BCBuild_Put(Builder, 'X');
    }

    Builder->EndInstPtr = Builder->Position;
    BCBuild_PutAddress(Builder, 0); // *****after***** last instruction
    Builder->StackPointerPos = Builder->Position;
    BCBuild_PutAddress(Builder, 0); // stack pointer
    BCBuild_Put(Builder, 0); // end of header

    // get the export count first
    size_t ExportCount;
    for (ExportCount = 0; ExportCount < (sizeof(Builder->Exports) / sizeof(Builder->Exports[0])); ExportCount++)
    {
        Export_t Export = Builder->Exports[ExportCount];
        if (Export.Name == NULL)
        {
            break;
        }
    }

    BCBuild_PutQWord(Builder, ExportCount);

    for (size_t i = 0; i < ExportCount; i++)
    {
        Export_t Export = Builder->Exports[i];
        BCBuild_Put(Builder, strlen(Export.Name));
        for (size_t j = 0; j < strlen (Export.Name); j++)
        {
            BCBuild_Put(Builder, Export.Name[j]);
        }
        BCBuild_PutAddress(Builder, Export.Address);
    }
}

#endif
