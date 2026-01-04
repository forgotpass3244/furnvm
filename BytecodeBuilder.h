
// BytecodeBuilder.h
// simple bytecode builder for furnvm bytecode programs

#ifndef BCBUILD_INCLUDE
#define BCBUILD_INCLUDE

#define MEMORY_C_DECL static
#include "furnvmsrc/Memory.h"

typedef struct
{
    const char *Name;
    QWord Address;
} Export_t;

typedef struct
{
    Memory *Mem;
    size_t Position;
    size_t InstructionsEnd;
    size_t EndInstPtr;
    size_t StackPointerPos;
    Export_t Exports[16];

} BytecodeBuilder;

void BCBuild_Put(BytecodeBuilder *Builder, Byte Value);
void BCBuild_PutQWord(BytecodeBuilder *Builder, QWord Value);
void BCBuild_PutAddress(BytecodeBuilder *Builder, QWord Value);
void BCBuild_EndInstructions(BytecodeBuilder *Builder);
void BCBuild_EmitFile(const BytecodeBuilder *Builder, FILE *f);
void BCBuild_Header(BytecodeBuilder *Builder);

#endif // BCBUILD_INCLUDE
