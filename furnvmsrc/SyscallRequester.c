
#include "../furnvm.h"
#include "SyscallRequester.h"

QWord SyscallRequester_Invoke(SyscallRequester *Requester, const QWord Number)
{
    return Kernel_InvokeSyscall(Requester->System, Requester->Machine, Requester->Mem, Number);
}
