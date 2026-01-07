

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../furnvm.h"
#include "Memory.h"
#include "Machine.h"
#include "SyscallRequester.h"
#include <unistd.h>
#include <signal.h>

#define KSTDOUT &Machine->SyscallRequest->System->Out

#define EXPECT_MAGIC(Char)                                                     \
    if (!Machine->IsRunning)                                                   \
    {                                                                          \
        return;                                                                \
    }                                                                          \
    Magic = Machine_FetchByte(Machine, Mem);                                   \
    if (Magic != Char)                                                         \
    {                                                                          \
        fmprintf(KSTDOUT, "header\n");                                         \
        Memory_DumpData(Mem, 0, 100);                                          \
        fmprintf(KSTDOUT, "not a valid furn program (expected '%i')\n", Char); \
        Machine_Abort(Machine);                                                \
    }                                                                          \
    Memory_WriteByte(Mem, (Memory_ReadQWord(Mem, INST_PTR) - 1), 0); // zero initialize

/*[[noreturn]] compiler wont stfu about this */ void Machine_Abort(Machine_t *Machine)
{
    fmprintf(KSTDOUT, "(aborting)\n");
    Machine->IsRunning = false;
}

void Machine_Free(Machine_t *Machine)
{
    if (Machine->SyscallRequest != NULL)
    {
        free(Machine->SyscallRequest);
    }
    for (size_t i = 0; i < (sizeof(Machine->Exports) / sizeof(Machine->Exports[0])); i++)
    {
        if (Machine->Exports[i].Name == NULL)
        {
            break;
        }
        free(Machine->Exports[i].Name);
    }
}

void Machine_Header(Machine_t *Machine, Memory *Mem)
{
    Machine->IsRunning = true; // run to read the header

    Machine->Flag_Zero = false;
    Machine->Flag_Equal = false;
    Machine->Flag_GreaterThan = false;
    Machine->EndInstruction = -1;

    // read header

    // instruction pointer
    Memory_ReadQWord(Mem, INST_PTR);

    Byte Magic;

    // magic
    EXPECT_MAGIC('f');
    EXPECT_MAGIC('u');
    EXPECT_MAGIC('r');
    EXPECT_MAGIC('n');

    QWord RegisterAllocSize = Machine_FetchQWord(Machine, Mem);

    for (size_t i = 0; i < RegisterAllocSize; i++)
    {
        EXPECT_MAGIC('X');
    }

    Machine->EndInstruction = Machine_FetchAddress(Machine, Mem); // EIP
    Machine->StackPointerPos = Memory_ReadQWord(Mem, INST_PTR); // stack pointer ptr
    Machine->StackBegin = Memory_ReadQWord(Mem, Machine->StackPointerPos);
    Machine_FetchAddress(Machine, Mem); // ignore the stack pointer init

    if (Machine_FetchByte(Machine, Mem) != 0)
    {
        fmprintf(KSTDOUT, "Invalid header end\n");
        Machine_Abort(Machine);
    }

    Memory_WriteQWord(Mem, Memory_ReadQWord(Mem, INST_PTR) - 1, Memory_ReadQWord(Mem, INST_PTR) - 1);

    // exports
    QWord ExportCount = Machine_FetchQWord(Machine, Mem);
    for (size_t i = 0; i < ExportCount; i++)
    {
        Byte Length = Machine_FetchByte(Machine, Mem);
        char Name[16] = {0};
        for (size_t j = 0; j < Length; j++)
        {
            Name[j] = Machine_FetchByte(Machine, Mem);
        }
        QWord Address = Machine_FetchQWord(Machine, Mem);
        Machine->Exports[i].Name = strdup(Name);
        Machine->Exports[i].Address = Address;
    }

    Machine->IsRunning = false;
}

void Machine_DumpState(const Machine_t *Machine, const Memory *Mem)
{
    fmprintf(KSTDOUT, "  State:\n");
    fmprintf(KSTDOUT, "    IP = %llu\n", Memory_ReadQWord(Mem, INST_PTR));
    fmprintf(KSTDOUT, "    [NULL] = 0x%06X\n", Memory_ReadByte(Mem, 0));

    fmprintf(KSTDOUT, "  Registers:\n");
    fmprintf(KSTDOUT, "    [...] byte  = %i\n", Memory_ReadByte(Mem, REGISTER_A));
    fmprintf(KSTDOUT, "    [...] byte  = %i\n", Memory_ReadByte(Mem, REGISTER_B));
    fmprintf(KSTDOUT, "    [...] byte  = %i\n", Memory_ReadByte(Mem, REGISTER_C));
    fmprintf(KSTDOUT, "    [...] byte  = %i\n", Memory_ReadByte(Mem, REGISTER_D));
    fmprintf(KSTDOUT, "    [...] byte  = %i\n", Memory_ReadByte(Mem, REGISTER_E));
    fmprintf(KSTDOUT, "    [...] qword = %llu\n", Memory_ReadQWord(Mem, REGISTER64_A));
    fmprintf(KSTDOUT, "    [...] qword = %llu\n", Memory_ReadQWord(Mem, REGISTER64_B));
    fmprintf(KSTDOUT, "    [...] qword = %llu\n", Memory_ReadQWord(Mem, REGISTER64_C));
    fmprintf(KSTDOUT, "    [...] qword = %llu\n", Memory_ReadQWord(Mem, REGISTER64_D));
    fmprintf(KSTDOUT, "    [...] qword = %llu\n", Memory_ReadQWord(Mem, REGISTER64_E));

    fmprintf(KSTDOUT, "  Flags:\n");
    fmprintf(KSTDOUT, "    Zero = %s\n", BOOL_AS_STR(Machine->Flag_Zero));
    fmprintf(KSTDOUT, "    Equal = %s\n", BOOL_AS_STR(Machine->Flag_Equal));

    fmprintf(KSTDOUT, "\n    Inst = %llu\n", Memory_ReadByte(Mem, Memory_ReadQWord(Mem, INST_PTR)));
}

Byte Machine_FetchByte(Machine_t *Machine, Memory *Mem)
{
    QWord IP = Memory_ReadQWord(Mem, INST_PTR);

    if (!Machine->IsRunning)
    {
        // fmprintf(KSTDOUT, "Invalid cycle\n");
        // Machine_Abort(Machine);
        return 0;
    }
    else if (IP >= Machine->EndInstruction)
    {
        fmprintf(KSTDOUT, "Instruction pointer out of bounds (bounded to %llu)\n", Machine->EndInstruction);
        Memory_DumpData(Mem, 0, 0);
        Machine_Abort(Machine);
    }

    Memory_IncQWord(Mem, INST_PTR);
    return Memory_ReadByte(Mem, IP);
}

QWord Machine_FetchQWord(Machine_t *Machine, Memory *Mem)
{
    QWord Result = 0;
    for (size_t i = 0; i < 8; i++)
    {
        Result |= ((QWord)(Machine_FetchByte(Machine, Mem)) << (i * 8));
    }
    return Result;
}

QWord Machine_FetchAddress(Machine_t *Machine, Memory *Mem)
{
    return Machine_FetchQWord(Machine, Mem);
}

void Machine_Execute(Machine_t *Machine, Memory *Mem)
{
    // system("clear");
    // Memory_DumpData(Mem, Machine->StackBegin, 640);
    // getchar(); // wait to press enter key

    Byte Instruction = Machine_FetchByte(Machine, Mem);

    if (Instruction > OPCODES_GREATEST)
        goto Label_InvalidInstruction;

#if OPCODES_GOTO_DISPATCH
    static void *InstructionDispatchTable[] = {
        [LOAD_BYTE] = &&Label_LOAD_BYTE,
        [LOAD_QWORD] = &&Label_LOAD_QWORD,
        [MOVE_QWORD] = &&Label_MOVE_QWORD,
        [DEREF_BYTE] = &&Label_DEREF_BYTE,
        [DEREF_QWORD] = &&Label_DEREF_QWORD,
        [SET_FLAGS_BYTE] = &&Label_SET_FLAGS_BYTE,
        [COMPARE_QWORD] = &&Label_COMPARE_QWORD,
        [COMPARE_BYTE] = &&Label_COMPARE_BYTE,
        [TICK_FLAGS] = &&Label_TICK_FLAGS,
        [DUMP_STATE] = &&Label_DUMP_STATE,
        [DUMP_VALUE_64] = &&Label_DUMP_VALUE_64,
        [DUMP_VALUE] = &&Label_DUMP_VALUE,
        [ADD_BYTE] = &&Label_ADD_BYTE,
        [SUB_BYTE] = &&Label_SUB_BYTE,
        [ADD_QWORD] = &&Label_ADD_QWORD,
        [SUB_QWORD] = &&Label_SUB_QWORD,
        [INC_QWORD] = &&Label_INC_QWORD,
        [DEC_QWORD] = &&Label_DEC_QWORD,
        [LOAD_EIP] = &&Label_LOAD_EIP,
        [CALL] = &&Label_CALL,
        [RETURN] = &&Label_RETURN,
        [JUMP] = &&Label_JUMP,
        [JUMP_IF_EQUAL] = &&Label_JUMP_IF_EQUAL,
        [JUMP_IF_GREATER] = &&Label_JUMP_IF_GREATER,
        [MAP_GREATER_BYTE] = &&Label_MAP_GREATER_BYTE,
        [JUMP_IF_ZERO] = &&Label_JUMP_IF_ZERO,
        [SYSCALL] = &&Label_SYSCALL,
        [MOVE_DYNAMIC] = &&Label_MOVE_DYNAMIC,
        [PUSH_QWORD] = &&Label_PUSH_QWORD,
        [POP_QWORD] = &&Label_POP_QWORD,
        [STACK_POINTER_FROM_OFFSET] = &&Label_STACK_POINTER_FROM_OFFSET,
        [STACK_WRITE_QWORD] = &&Label_STACK_WRITE_QWORD,
        [STACK_READ_QWORD] = &&Label_STACK_READ_QWORD,
    };

    goto *InstructionDispatchTable[Instruction];
#endif

    // wrapped in a switch so breaks can skip over the other instructions
    switch (Instruction)
    {
    case LOAD_BYTE: // load a byte into a register
    {
    Label_LOAD_BYTE:
        QWord Address = Machine_FetchAddress(Machine, Mem);
        Byte Value = Machine_FetchByte(Machine, Mem);
        Memory_WriteByte(Mem, Address, Value);
    }
    break;

    case LOAD_QWORD: // load a qword into a register64
    {
    Label_LOAD_QWORD:
        QWord Address = Machine_FetchAddress(Machine, Mem);
        QWord Value = Machine_FetchQWord(Machine, Mem);
        Memory_WriteQWord(Mem, Address, Value);
    }
    break;

    case MOVE_QWORD: // dereferences for you
    {
    Label_MOVE_QWORD:
        QWord Source = Machine_FetchAddress(Machine, Mem);
        QWord Destination = Machine_FetchAddress(Machine, Mem);
        Memory_WriteQWord(Mem, Destination, Memory_ReadQWord(Mem, Source));
    }
    break;

    case MOVE_DYNAMIC:
    {
    Label_MOVE_DYNAMIC:
        QWord Source = Machine_FetchAddress(Machine, Mem);
        Byte SourceSize = Machine_FetchByte(Machine, Mem);
        QWord Destination = Machine_FetchAddress(Machine, Mem);
        Byte DestinationSize = Machine_FetchByte(Machine, Mem);

        uint64_t Value = 0;

        for (Byte i = 0; i < SourceSize; i++)
        {
            Value |= (uint64_t)Memory_ReadByte(Mem, Source + i) << (i * 8);
        }

        for (Byte i = 0; i < DestinationSize; i++)
        {
            Memory_WriteByte(
                Mem,
                Destination + i,
                (Byte)((Value >> (i * 8)) & 0xFF));
        }
    }
    break;
    
    case DEREF_BYTE: // memory random access read and put back into register
    {
    Label_DEREF_BYTE:
        QWord Source = Machine_FetchAddress(Machine, Mem);
        QWord Destination = Machine_FetchAddress(Machine, Mem);
        QWord AddressToDeref = Memory_ReadQWord(Mem, Source);
        Memory_WriteByte(Mem, Destination, Memory_ReadByte(Mem, AddressToDeref));
    }
    break;

    case DEREF_QWORD: // memory random access read and put back into register
    {
    Label_DEREF_QWORD:
        QWord Source = Machine_FetchAddress(Machine, Mem);
        QWord Destination = Machine_FetchAddress(Machine, Mem);
        QWord AddressToDeref = Memory_ReadQWord(Mem, Source);
        Memory_WriteQWord(Mem, Destination, Memory_ReadQWord(Mem, AddressToDeref));
    }
    break;

    case SET_FLAGS_BYTE: // set flags based on a register
    {
    Label_SET_FLAGS_BYTE:
        QWord Address = Machine_FetchAddress(Machine, Mem);
        Machine->Flag_Zero = (Memory_ReadByte(Mem, Address) == 0);
    }
    break;

    case COMPARE_QWORD: // compare qwords and set flags
    {
    Label_COMPARE_QWORD:
        QWord FirstRegister = Machine_FetchAddress(Machine, Mem);
        QWord SecondRegister = Machine_FetchAddress(Machine, Mem);

        QWord FirstValue = Memory_ReadQWord(Mem, FirstRegister);
        QWord SecondValue = Memory_ReadQWord(Mem, SecondRegister);

        Machine->Flag_Equal = (FirstValue == SecondValue);
        Machine->Flag_GreaterThan = (FirstValue > SecondValue);
    }
    break;

    case COMPARE_BYTE: // compare bytes and set flags
    {
    Label_COMPARE_BYTE:
        QWord FirstRegister = Machine_FetchAddress(Machine, Mem);
        QWord SecondRegister = Machine_FetchAddress(Machine, Mem);

        QWord FirstValue = Memory_ReadByte(Mem, FirstRegister);
        QWord SecondValue = Memory_ReadByte(Mem, SecondRegister);

        Machine->Flag_Equal = (FirstValue == SecondValue);
        Machine->Flag_GreaterThan = (FirstValue > SecondValue);
    }
    break;

    case DUMP_STATE: // dump machine state
    {
    Label_DUMP_STATE:
        Machine_DumpState(Machine, Mem);
    }
    break;

    case DUMP_VALUE_64: // dump register64
    {
    Label_DUMP_VALUE_64:
        QWord Address = Machine_FetchAddress(Machine, Mem);
        QWord Value = Memory_ReadQWord(Mem, Address);
        fmprintf(KSTDOUT, "[%03i] qword = %llu\n", Address, Value);
    }
    break;

    case DUMP_VALUE: // dump register
    {
    Label_DUMP_VALUE:
        QWord Address = Machine_FetchAddress(Machine, Mem);
        Byte Value = Memory_ReadByte(Mem, Address);
        fmprintf(KSTDOUT, "[%03i] byte  = %llu\n", Address, Value);
    }
    break;

    case ADD_BYTE: // add two byte registers and store into the destination
    {
    Label_ADD_BYTE:
        QWord DestinationRegister = Machine_FetchAddress(Machine, Mem);
        QWord FirstRegister = Machine_FetchAddress(Machine, Mem);
        QWord SecondRegister = Machine_FetchAddress(Machine, Mem);

        Memory_WriteByte
        (
            Mem,
            DestinationRegister,
            Memory_ReadByte(Mem, FirstRegister)
            + Memory_ReadByte(Mem, SecondRegister)
        );
    }
    break;

    case SUB_BYTE: // sub two byte registers and store into the destination
    {
    Label_SUB_BYTE:
        QWord DestinationRegister = Machine_FetchAddress(Machine, Mem);
        QWord FirstRegister = Machine_FetchAddress(Machine, Mem);
        QWord SecondRegister = Machine_FetchAddress(Machine, Mem);

        Memory_WriteByte(
            Mem,
            DestinationRegister,
            Memory_ReadByte(Mem, FirstRegister) - Memory_ReadByte(Mem, SecondRegister)
        );
    }
    break;

    case ADD_QWORD: // add two qword registers and store into the destination
    {
    Label_ADD_QWORD:
        QWord DestinationRegister = Machine_FetchAddress(Machine, Mem);
        QWord FirstRegister = Machine_FetchAddress(Machine, Mem);
        QWord SecondRegister = Machine_FetchAddress(Machine, Mem);

        Memory_WriteQWord(
            Mem,
            DestinationRegister,
            Memory_ReadQWord(Mem, FirstRegister) + Memory_ReadQWord(Mem, SecondRegister));
    }
    break;

    case SUB_QWORD: // sub two qword registers and store into the destination
    {
    Label_SUB_QWORD:
        QWord DestinationRegister = Machine_FetchAddress(Machine, Mem);
        QWord FirstRegister = Machine_FetchAddress(Machine, Mem);
        QWord SecondRegister = Machine_FetchAddress(Machine, Mem);

        Memory_WriteQWord(
            Mem,
            DestinationRegister,
            Memory_ReadQWord(Mem, FirstRegister) - Memory_ReadQWord(Mem, SecondRegister));
    }
    break;

    case INC_QWORD:
    {
    Label_INC_QWORD:
        QWord Address = Machine_FetchAddress(Machine, Mem);
        Memory_IncQWord(Mem, Address);
    }
    break;

    case DEC_QWORD:
    {
    Label_DEC_QWORD:
        QWord Address = Machine_FetchAddress(Machine, Mem);
        Memory_DecQWord(Mem, Address);
    }
    break;

    case LOAD_EIP: // put EIP into a register
    {
    Label_LOAD_EIP:
        QWord Address = Machine_FetchAddress(Machine, Mem);

        Memory_WriteByte(
            Mem,
            Address,
            Machine->EndInstruction
        );
    }
    break;

    case CALL: // jump to address then push return addr to the return stack
    {
    Label_CALL:
        Machine->ReturnStack[++Machine->ReturnStackPos] = (Memory_ReadQWord(Mem, INST_PTR) + sizeof(QWord));

        QWord Address = Machine_FetchAddress(Machine, Mem);

        Memory_WriteQWord(
            Mem,
            INST_PTR,
            Address);
    }
    break;

    case RETURN: // pop off the return stack and jump there
    {
    Label_RETURN:
        if (Machine->ReturnStackPos <= 0)
        {
            fmprintf(KSTDOUT, "Return stack underflow\n");
            Machine_Abort(Machine);
            return;
        }
        QWord ReturnAddress = Machine->ReturnStack[Machine->ReturnStackPos--];

        Memory_WriteQWord(
            Mem,
            INST_PTR,
            ReturnAddress);
    }
    break;

    case JUMP: // jump to address in register
    {
    Label_JUMP:
        QWord Address = Machine_FetchAddress(Machine, Mem);

        Memory_WriteQWord(
            Mem,
            INST_PTR,
            Address);
    }
    break;

    case JUMP_IF_EQUAL: // jump to address in register if equal flag is true
    {
    Label_JUMP_IF_EQUAL:
        QWord Address = Machine_FetchAddress(Machine, Mem);

        Memory_WriteQWord(
            Mem,
            INST_PTR,
            (Machine->Flag_Equal ? Address : Memory_ReadQWord(Mem, INST_PTR))
        );
    }
    break;

    case MAP_GREATER_BYTE:
    {
    Label_MAP_GREATER_BYTE:
        QWord Address = Machine_FetchAddress(Machine, Mem);

        Memory_WriteByte(
            Mem,
            Address,
            Machine->Flag_GreaterThan);
    }
    break;

    case JUMP_IF_GREATER: // jump to address in register if greaterthan flag is true
    {
    Label_JUMP_IF_GREATER:
        QWord Address = Machine_FetchAddress(Machine, Mem);

        Memory_WriteQWord(
            Mem,
            INST_PTR,
            (Machine->Flag_GreaterThan ? Address : Memory_ReadQWord(Mem, INST_PTR)));
    }
    break;

    case JUMP_IF_ZERO: // jump to address in register if zero flag is true
    {
    Label_JUMP_IF_ZERO:
        QWord Address = Machine_FetchAddress(Machine, Mem);

        Memory_WriteQWord(
            Mem,
            INST_PTR,
            (Machine->Flag_Zero ? Address : Memory_ReadQWord(Mem, INST_PTR)));
    }
    break;

    case TICK_FLAGS: // flip equal flag
    {
    Label_TICK_FLAGS:
        // evil black magic
        Machine->Flag_Equal ^= true;
        Machine->Flag_GreaterThan ^= true;
        Machine->Flag_Zero ^= true;
    }
    break;

    case PUSH_QWORD: // write to the stack then increment stack pointer
    {
    Label_PUSH_QWORD:
        QWord StackPtr = Memory_ReadQWord(Mem, Machine->StackPointerPos);
        QWord Source = Machine_FetchAddress(Machine, Mem);

        // write to stack
        Memory_WriteQWord(
            Mem,
            StackPtr,
            Memory_ReadQWord(Mem, Source)
        );

        // increment stack pointer
        Memory_WriteQWord(
            Mem,
            Machine->StackPointerPos,
            Memory_ReadQWord(Mem, Machine->StackPointerPos) + sizeof(QWord)
        );

        // fprintf(stderr, "PUSH_QWORD: wrote %llu to stack at %llu, new SP %llu\n",
        //     Memory_ReadQWord(Mem, Source),
        //     StackPtr,
        //     Memory_ReadQWord(Mem, Machine->StackPointerPos));
    }
    break;

    case POP_QWORD: // take the top of the stack and put its value into a register, then decrement the stack pointer
    {
    Label_POP_QWORD:
        QWord StackPtr = Memory_ReadQWord(Mem, Machine->StackPointerPos);
        QWord Destination = Machine_FetchAddress(Machine, Mem);
        
        // write to destination
        if (Destination)
        {
            Memory_WriteQWord(
                Mem,
                Destination,
                Memory_ReadQWord(Mem, StackPtr - /*important*/ sizeof(QWord)));
        }
            
        // decrement stack pointer
        Memory_WriteQWord(
            Mem,
            Machine->StackPointerPos,
            StackPtr - sizeof(QWord));

        // fprintf(stderr, "POP_QWORD: read %llu from stack at %llu, new SP %llu\n",
        //     Memory_ReadQWord(Mem, StackPtr - sizeof(QWord)),
        //     StackPtr,
        //     Memory_ReadQWord(Mem, Machine->StackPointerPos));
    }
    break;

    case STACK_POINTER_FROM_OFFSET:
    {
    Label_STACK_POINTER_FROM_OFFSET:
        QWord Offset = Machine_FetchQWord(Machine, Mem);
        QWord Destination = Machine_FetchAddress(Machine, Mem);
        QWord LocOnStack = (Memory_ReadQWord(Mem, Machine->StackPointerPos) - Offset);
        Memory_WriteQWord(Mem, Destination, LocOnStack);
    }
    break;

    case STACK_WRITE_QWORD:
    {
    Label_STACK_WRITE_QWORD:
        QWord Offset = Machine_FetchQWord(Machine, Mem);
        QWord Source = Machine_FetchAddress(Machine, Mem);
        QWord LocOnStack = (Memory_ReadQWord(Mem, Machine->StackPointerPos) - Offset);
        Memory_WriteQWord(Mem, LocOnStack, Memory_ReadQWord(Mem, Source));
    }
    break;

    case STACK_READ_QWORD:
    {
    Label_STACK_READ_QWORD:
        QWord Offset = Machine_FetchQWord(Machine, Mem);
        QWord Destination = Machine_FetchAddress(Machine, Mem);
        QWord LocOnStack = (Memory_ReadQWord(Mem, Machine->StackPointerPos) - Offset);
        Memory_WriteQWord(Mem, Destination, Memory_ReadQWord(Mem, LocOnStack));
    }
    break;

    case SYSCALL: // syscall
    {
    Label_SYSCALL:
        Byte SyscallNumber = Machine_FetchByte(Machine, Mem);
        QWord ResultRegister = Machine_FetchAddress(Machine, Mem);
        QWord Result = SyscallRequester_Invoke(Machine->SyscallRequest, SyscallNumber);
        
        if (ResultRegister)
        {
            Memory_WriteQWord(Mem, ResultRegister, Result);
        }
    }
    break;

    default:
    {
#if !OPCODES_GOTO_DISPATCH
        goto Label_InvalidInstruction;
        // silence unused labels
        // kinda hacky but it is what it is
        goto Label_LOAD_BYTE;
        goto Label_LOAD_QWORD;
        goto Label_MOVE_QWORD;
        goto Label_DEREF_BYTE;
        goto Label_DEREF_QWORD;
        goto Label_SET_FLAGS_BYTE;
        goto Label_COMPARE_QWORD;
        goto Label_COMPARE_BYTE;
        goto Label_TICK_EQUAL;
        goto Label_DUMP_STATE;
        goto Label_DUMP_VALUE_64;
        goto Label_DUMP_VALUE;
        goto Label_ADD_BYTE;
        goto Label_SUB_BYTE;
        goto Label_ADD_QWORD;
        goto Label_LOAD_EIP;
        goto Label_JUMP;
        goto Label_JUMP_IF_EQUAL;
        goto Label_SYSCALL;
        goto Label_MOVE_DYNAMIC;
#endif
    Label_InvalidInstruction:
        fmprintf(KSTDOUT, "Invalid instruction 0x%06X at IP %llu\n", Instruction, Memory_ReadQWord(Mem, INST_PTR) - 1);
        Machine_Abort(Machine);
    }
    break;
    }
}

void Machine_ExecuteCycles(Machine_t *Machine, Memory *Mem)
{
    Machine->IsRunning = true;
    while (Machine->IsRunning)
    {
        Machine_Execute(Machine, Mem);
    }
}
