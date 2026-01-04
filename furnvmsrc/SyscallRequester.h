
#ifndef SYSCALLREQUESTER_H
#define SYSCALLREQUESTER_H

#include "Machine.h"
#include "System.h"
#include "Memory.h"

typedef struct SyscallRequester SyscallRequester;

struct SyscallRequester
{
    System_t *System;
    Machine_t *Machine;
    Memory *Mem;
};

QWord SyscallRequester_Invoke(SyscallRequester *Requester, const QWord Number);

#endif // SYSCALLREQUESTER_H
