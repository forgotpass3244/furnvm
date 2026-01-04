
#ifndef DISASSEMBLER_H
#define DISASSEMBLER_H

#include "furnvmsrc/File.h"

// this function is kinda broken currently
// spits out garbage instructions until it realigns
void DisassembleExecutable(Memory *restrict Executable, File_t *Out);

#endif // DISASSEMBLER_H

