
#ifndef SYSTEM_H
#define SYSTEM_H

typedef struct System_t System_t;

#include "Machine.h"
#include "Memory.h"
#include "File.h"
#include "Directory.h"

typedef struct MachineNode MachineNode;

struct MachineNode {
    // linked list
    Machine_t *Machine;
    Memory *Mem;
    MachineNode *Next;
};

void MachineNode_Append(MachineNode *List, MachineNode *NewNode);

typedef struct
{
    void (*FlushCallback)(Byte *Data, size_t Length);
} SystemConfig_t;

struct System_t
{
    SystemConfig_t Config;
    MachineNode *List;
    Directory_t RootDir;
    File_t Out;
    size_t LastFlushed;
};

void System_Init(System_t *System);

void System_Free(System_t *System);

void System_RemoveMachines(System_t *System);

void System_AddMachine(System_t *System, Machine_t *Machine, Memory *Mem);

void System_ExecuteAll(System_t *System);

QWord System_InvokeSyscall(System_t *System, Machine_t *Machine, Memory *Mem, const QWord Number);

QWord Sys_FlushOut(System_t *System, Machine_t *Machine, Memory *Mem);

size_t System_SplitArgs(System_t *System, const char *String, char *Out);

void System_DumpLs(System_t *System, const Directory_t *Dir);

#endif // SYSTEM_H
