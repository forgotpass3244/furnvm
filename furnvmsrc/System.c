
#include "Machine.h"
#include "Memory.h"
#include "System.h"
#include "SyscallRequester.h"
#include "File.h"
#include <stdio.h>

typedef QWord (*SyscallFunction)(System_t *System, Machine_t *Machine, Memory *Mem);

static QWord Sys_Exit(System_t *System, Machine_t *Machine, Memory *Mem)
{
    (void)System;
    (void)Mem;

    Machine->IsRunning = false;
    return 0;
}

QWord Sys_FlushOut(System_t *System, Machine_t *Machine, Memory *Mem)
{
    (void)Machine;
    (void)Mem;

    File_t *Out = &System->Out;
    
    // handle reset / truncate
    if (Out->Position < System->LastFlushed)
        System->LastFlushed = 0;

    if (Out->Position > System->LastFlushed)
    {
        if (System->Config.FlushCallback != NULL)
        {
            size_t Diff = Out->Position - System->LastFlushed;
            System->Config.FlushCallback(
                &Out->Mem.Data[System->LastFlushed],
                Diff);
        }

        System->LastFlushed = Out->Position;
    }

    return 0;
}

static QWord Sys_WriteOut(System_t *System, Machine_t *Machine, Memory *Mem)
{
    (void)Machine;

    QWord Pointer = Memory_ReadQWord(Mem, SYSCALL_ARG1);
    QWord Length = Memory_ReadQWord(Mem, SYSCALL_ARG2);

    File_Write(&System->Out, &Mem->Data[Pointer], Length);

    for (size_t i = 0; i < Length; i++)
    {
        if (Mem->Data[Pointer + i] == '\n')
        {
            Sys_FlushOut(System, NULL, NULL);
        }
    }

    return 0;
}


static SyscallFunction SyscallTable[] = {
    Sys_Exit,
    Sys_WriteOut,
    Sys_FlushOut,
};

void MachineNode_Append(MachineNode *List, MachineNode *NewNode)
{
    MachineNode *Node = List;
    while (Node->Next)
    {
        Node = Node->Next;
    }
    Node->Next = NewNode;
}


void System_Init(System_t *System)
{
    System->List = NULL;
    Memory_Zero(&System->Out.Mem);
    System->LastFlushed = 0;
    System->RootDir.Ls = NULL;
    System->RootDir.Subdirs = NULL;
    System->RootDir.Parent = NULL;
    System->Config = (SystemConfig_t) {0};
    Memory_Zero(&System->Out.Mem);
}

void System_Free(System_t *System)
{
    File_Close(&System->Out);

    System_RemoveMachines(System);

    LsNode *Node = System->RootDir.Ls;
    while (Node)
    {
        LsNode *OldNode = Node;
        Node = OldNode->Next;
        free(OldNode->File);
        free(OldNode);
    }
}

void System_RemoveMachines(System_t *System)
{
    MachineNode *Node = System->List;
    System->List = NULL;
    while (Node)
    {
        MachineNode *OldNode = Node;
        Node = OldNode->Next;
        free(OldNode);
    }
}

void System_AddMachine(System_t *System, Machine_t *Machine, Memory *Mem)
{
    MachineNode *Node = malloc(sizeof(MachineNode));
    Node->Machine = Machine;
    Node->Mem = Mem;
    Node->Next = NULL;

    Machine->SyscallRequest = malloc(sizeof(SyscallRequester));
    Machine->SyscallRequest->System = System;
    Machine->SyscallRequest->Machine = Machine;
    Machine->SyscallRequest->Mem = Mem;

    if (System->List == NULL)
    {
        System->List = Node;
    }
    else
    {
        MachineNode_Append(System->List, Node);
    }
}

void System_ExecuteAll(System_t *System)
{
    MachineNode *Node = System->List;
    while (Node)
    {
        Machine_t *Machine = Node->Machine;
        Memory *Mem = Node->Mem;

        Machine_Header(Machine, Mem);
        Machine_ExecuteCycles(Machine, Mem);

        Node = Node->Next;
    }
}

QWord System_InvokeSyscall(System_t *System, Machine_t *Machine, Memory *Mem, const QWord Number)
{
    (void)System;
    if (Number < sizeof(SyscallTable) / sizeof(SyscallFunction))
    {
        SyscallFunction Function = SyscallTable[Number];
        return Function(System, Machine, Mem);
    }
    else
    {
        Machine_Abort(Machine);
        return 0;
    }
}

size_t System_SplitArgs(System_t *System, const char *String, char *Out)
{
    (void)System;

    size_t argc = 1;
    for (size_t i = 0; i < strlen(String); i++)
    {
        char c = String[i];
        if (c == ' ')
        {
            Out[i] = '\0';
            argc++;
        }
        else
        {
            Out[i] = c;
        }
    }

    return argc;
}

void System_DumpLs(System_t *System, const Directory_t *Dir)
{
    {
        LsNode *Node = Dir->Ls;
        while (Node)
        {
            fmprintf(&System->Out, "%s ", Node->Name);
            Node = Node->Next;
        }
    }

    {
        SubdirNode *Node = Dir->Subdirs;
        while (Node)
        {
            fmprintf(&System->Out, "%s/ ", Node->Name);
            Node = Node->Next;
        }
    }

    fmprintf(&System->Out, "\n");
}

