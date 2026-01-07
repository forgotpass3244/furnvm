
// furnvm.h
// definitions for libfurnvm
// the library actually depends on this header itself
// so do not move this file (you can still copy it though)

#ifndef FURNVM_H
#define FURNVM_H

#include <stdbool.h>
#define BOOL_AS_STR(Expr) ((Expr) ? "true" : "false")

// WORD types
typedef bool Bit;
typedef unsigned char Byte;
typedef unsigned short Word;
typedef unsigned int DWord;
typedef unsigned long long QWord;

#define PROGRAM_SIZE (1024 * 8)

#define INST_PTR 0
#define REGISTER_A 8
#define REGISTER_B 9
#define REGISTER_C 10
#define REGISTER_D 11
#define REGISTER_E 12
#define REGISTER64_A 13
#define REGISTER64_B 21
#define REGISTER64_C 29
#define REGISTER64_D 37
#define REGISTER64_E 45

#define REGISTER_ALLOC_SIZE 12

// disable if not using clang or gcc
#define OPCODES_GOTO_DISPATCH 1

// opcodes
#define LOAD_BYTE          0
#define LOAD_QWORD         1
#define DEREF_BYTE         2
#define DEREF_QWORD        3
#define SET_FLAGS_BYTE     4
#define COMPARE_BYTE       5
#define DUMP_STATE         6
#define DUMP_VALUE_64      7
#define DUMP_VALUE         8
#define ADD_BYTE           9
#define SUB_BYTE           10
#define ADD_QWORD          11
#define LOAD_EIP           12
#define JUMP               13
#define JUMP_IF_EQUAL      14
#define SYSCALL            15
#define COMPARE_QWORD      16
#define TICK_FLAGS         17
#define MOVE_QWORD         18
#define MOVE_DYNAMIC       19
#define CALL               20
#define RETURN             21
#define JUMP_IF_ZERO       22
#define SUB_QWORD          23
#define PUSH_QWORD         24
#define POP_QWORD          25
#define STACK_POINTER_FROM_OFFSET 26
#define STACK_WRITE_QWORD  27
#define STACK_READ_QWORD   28
#define INC_QWORD          29
#define JUMP_IF_GREATER    30
#define MAP_GREATER_BYTE   31
#define DEC_QWORD          32
#define OPCODES_GREATEST DEC_QWORD

// syscall registers
#define SYSCALL_ARG1       REGISTER64_A
#define SYSCALL_ARG2       REGISTER64_B
#define SYSCALL_ARG3       REGISTER64_C
#define SYSCALL_ARG4       REGISTER64_D

// syscall numbers
#define SYSNUM_EXIT        0
#define SYSNUM_WRITE_OUT   1

#ifndef NO_INCLUDE_FURNVM_HEADERS_FLAG
#include "furnvmsrc/System.h" // the source code of furnvm
#endif

#endif // FURNVM_H
