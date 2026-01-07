
#include "Disassembler.h"
#include "furnvmsrc/File.h"

#define ADDR_AS_STR(Address)    (Address == REGISTER_A ? "ra8" : \
                                 Address == REGISTER_B ? "rb8" : \
                                 Address == REGISTER_C ? "rc8" : \
                                 Address == REGISTER_D ? "rd8" : \
                                 Address == REGISTER64_A ? "ra64" : \
                                 Address == REGISTER64_B ? "rb64" : \
                                 Address == REGISTER64_C ? "rc64" : \
                                 Address == REGISTER64_D ? "rd64" : \
                                 "addr_some")

void DisassembleExecutable(Memory *restrict Executable, File_t *Out)
{
    QWord RegisterAllocSize = Memory_ReadQWord(Executable, REGISTER_ALLOC_SIZE);
    QWord EIP = Memory_ReadQWord(Executable, (REGISTER_ALLOC_SIZE + RegisterAllocSize + 8));

    Memory_DumpData(Executable, 0, 600);

    for (size_t i = (REGISTER_ALLOC_SIZE + RegisterAllocSize + 17); i < (EIP - 1);)
    {
        fmprintf(Out, "  &%llu: ", i);

        Byte Opcode = Memory_ReadByte(Executable, i);
        i += sizeof(Byte);

        switch (Opcode)
        {
        case LOAD_BYTE:
        {
            QWord Address = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            Byte Value = Memory_ReadByte(Executable, i);
            i += sizeof(Byte);
            fmprintf(Out, "load8 %llu, %u\n", Address, Value);
        }
        break;

        case LOAD_QWORD:
        {
            QWord Address = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            QWord Value = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "load64 %llu, %lu\n", Address, Value);
        }
        break;

        case SYSCALL:
        {
            Byte Number = Memory_ReadByte(Executable, i);
            i += sizeof(Byte);
            QWord ResultAddress = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "syscall %i -> %llu\n", Number, ResultAddress);
        }
        break;

        case JUMP:
        {
            QWord Address = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "jump %llu\n", Address);
        }
        break;

        case JUMP_IF_EQUAL:
        {
            QWord Address = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "jumpeq %llu\n", Address);
        }
        break;

        case JUMP_IF_ZERO:
        {
            QWord Address = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "jumpifz %llu\n", Address);
        }
        break;

        case SET_FLAGS_BYTE:
        {
            QWord Address = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "setflags8 %llu\n", Address);
        }
        break;

        case MOVE_QWORD:
        {
            QWord Source = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            QWord Destination = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "move64 %llu -> %llu\n", Source, Destination);
        }
        break;

        case MOVE_DYNAMIC:
        {
            QWord Source = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            Byte SourceSize = Memory_ReadByte(Executable, i);
            i += sizeof(Byte);
            QWord Destination = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            Byte DestinationSize = Memory_ReadByte(Executable, i);
            i += sizeof(Byte);
            fmprintf(Out, "movedyn %llu, %u -> %llu, %u\n", Source, SourceSize, Destination, DestinationSize);
        }
        break;

        case RETURN:
        {
            fmprintf(Out, "return\n");
        }
        break;

        case CALL:
        {
            QWord Address = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "call %llu\n", Address);
        }
        break;

        case ADD_QWORD:
        {
            QWord Destination = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            QWord FirstRegister = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            QWord SecondRegister = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "add64 %llu, %llu -> %llu\n", FirstRegister, SecondRegister, Destination);
        }
        break;

        case SUB_QWORD:
        {
            QWord Destination = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            QWord FirstRegister = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            QWord SecondRegister = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "sub64 %llu, %llu -> %llu\n", FirstRegister, SecondRegister, Destination);
        }
        break;

        case INC_QWORD:
        {
            QWord Address = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "inc64 %llu\n", Address);
        }
        break;

        case DEREF_QWORD:
        {
            QWord Register = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "deref64 %llu\n", Register);
        }
        break;

        case PUSH_QWORD:
        {
            QWord Source = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "push64 %llu\n", Source);
        }
        break;

        case POP_QWORD:
        {
            QWord Destination = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "pop64 -> %llu\n", Destination);
        }
        break;

        case STACK_POINTER_FROM_OFFSET:
        {
            QWord Offset = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            QWord Destination = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "stackptrfo %llu -> %llu\n", Offset, Destination);
        }
        break;

        case STACK_WRITE_QWORD:
        {
            QWord Offset = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            QWord Source = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "stackwrite64 %llu -> %llu\n", Source, Offset);
        }
        break;

        case STACK_READ_QWORD:
        {
            QWord Offset = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            QWord Destination = Memory_ReadQWord(Executable, i);
            i += sizeof(QWord);
            fmprintf(Out, "stackread64 %llu -> %llu\n", Offset, Destination);
        }
        break;

        case DUMP_STATE:
        {
            fmprintf(Out, "dumpstate\n");
        }
        break;

        default:
        {
            fmprintf(Out, "Unknown opcode 0x%06X\n", Opcode);
        }
        break;
        }
    }
}
