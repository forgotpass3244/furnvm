
#ifndef MACHINE_H
#define MACHINE_H

typedef struct Machine_t Machine_t;

#include <stdio.h>
#include <stdlib.h>
#include "Memory.h"
#include "../furnvm.h"
#include "SyscallRequester.h"

typedef struct SyscallRequester SyscallRequester;

typedef struct
{
    char *Name;
    QWord Address;
} ExportedSymbol_t;

struct Machine_t
{
    // state
    bool IsRunning;
    QWord EndInstruction;
    QWord StackPointerPos;
    QWord StackBegin;
    QWord ReturnStack[64];
    size_t ReturnStackPos;

    // flags
    Bit Flag_Zero;
    Bit Flag_Equal;
    Bit Flag_GreaterThan;

    // kernel connection
    SyscallRequester *restrict SyscallRequest;
    ExportedSymbol_t Exports[20];
};

/*[[noreturn]]*/ void Machine_Abort(Machine_t *Machine);

void Machine_Free(Machine_t *Machine);

void Machine_Header(Machine_t *Machine, Memory *Mem);

void Machine_DumpState(const Machine_t *Machine, const Memory *Mem);

Byte Machine_FetchByte(Machine_t *Machine, Memory *Mem);

QWord Machine_FetchQWord(Machine_t *Machine, Memory *Mem);

QWord Machine_FetchAddress(Machine_t *Machine, Memory *Mem);

void Machine_Execute(Machine_t *Machine, Memory *Mem);

void Machine_ExecuteCycles(Machine_t *Machine, Memory *Mem);

#endif // MACHINE_H
