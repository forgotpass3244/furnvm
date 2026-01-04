

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>

#include "furnvm.h"
#include "Disassembler.h"

Byte ExecuteNonvirtualFile(const char *Name, System_t *System)
{
    FILE *f = fopen(Name, "rb");
    if (f == NULL)
    {
        fmprintf(&System->Out, "failed to find the file '%s'\n", Name);
        return 1;
    }

    Memory Mem;
    Memory_Zero(&Mem);
    Memory_FileRead(&Mem, f);
    fclose(f);

    Machine_t Machine = {0};
    Kernel_AddMachine(System, &Machine, &Mem);

    Kernel_ExecuteAll(System);
    Kernel_RemoveMachines(System);
    Machine_Free(&Machine);

    Byte ExitCode = Memory_ReadByte(&Mem, REGISTER_A);
    return ExitCode;
}

Byte ExecuteVirtualFile(const char *Name, System_t *System, Directory_t *CurrentWorkingDir)
{
    File_t *File = Directory_FindFileByRelativePath(CurrentWorkingDir, Name);
    if (File == NULL)
    {
        fmprintf(&System->Out, "failed to find the file '%s'\n", Name);
        return 1;
    }

    Memory Mem = File->Mem; // copy file memory to a new sandbox
    File_Close(File);

    Machine_t Machine = {0};
    Kernel_AddMachine(System, &Machine, &Mem);

    Kernel_ExecuteAll(System);
    Kernel_RemoveMachines(System);
    Machine_Free(&Machine);

    Byte ExitCode = Memory_ReadByte(&Mem, REGISTER_A);
    return ExitCode;
}

int REPL(System_t *System)
{
    const char *DirPath = "./virtual";
    DIR *Dir = opendir(DirPath);
    if (Dir == NULL)
    {
        // perror("opendir");
        // return EXIT_FAILURE;
    }

    Directory_t *MyDir = malloc(sizeof(Directory_t));
    MyDir->Ls = NULL;
    MyDir->Parent = NULL;
    MyDir->Subdirs = NULL;

    if (Dir)
    {
        struct dirent *Entry;
        while ((Entry = readdir(Dir)) != NULL)
        {
            // Skip "." and ".."
            if (Entry->d_name[0] == '.' &&
                (Entry->d_name[1] == '\0' ||
                 (Entry->d_name[1] == '.' && Entry->d_name[2] == '\0')))
                continue;

            char Name[56] = "./virtual/";
            strcat(Name, Entry->d_name);

            FILE *f = fopen(Name, "rb");

            File_t *MyFile = malloc(sizeof(File_t));
            MyFile->Position = 0;
            Memory_FileRead(&MyFile->Mem, f);

            fclose(f);

            Directory_AddFile(MyDir, MyFile, strdup(Entry->d_name));
        }

        closedir(Dir);
    }

    Directory_AddSubdir(&System->RootDir, MyDir, "virtual");

    Byte ExitCode = 0;
    Directory_t *CurrentWorkingDir = MyDir;
    while (true)
    {
        char Buffer[56];
        fmprintf(&System->Out, "furnVM >> ");
        Sys_FlushOut(System, NULL, NULL);

        if (fgets(Buffer, sizeof(Buffer), stdin) == NULL)
        {
            continue;
        }
        Buffer[strcspn(Buffer, "\n")] = 0;

        if (strlen(Buffer) == 0)
        {
            continue;
        }
        else if (strcmp(Buffer, "!") == 0)
        {
            break;
        }
        else if (strcmp(Buffer, "?") == 0)
        {
            // print last exit code to kernel out
            fmprintf(&System->Out, "exitcode %u\n", ExitCode);
        }
        else
        {
            char Argv[56] = {0};
            char *pArgv = Argv;
            size_t Argc = Kernel_SplitArgs(System, Buffer, pArgv);
            if (Argc <= 0)
            {
                continue;
            }
            else if (Argc == 1)
            {
                if (strcmp(pArgv, "ls") == 0)
                {
                    Kernel_DumpLs(System, CurrentWorkingDir);
                }
                else
                {
                    ExitCode = ExecuteVirtualFile(pArgv, System, CurrentWorkingDir);
                }
            }
            else
            {
                if (strcmp(pArgv, "cd") == 0)
                {
                    pArgv += strlen(pArgv) + 1; // move to the next argument
                    Directory_t *NewDir = Directory_FindDirByRelativePath(CurrentWorkingDir, pArgv);
                    if (NewDir)
                    {
                        CurrentWorkingDir = NewDir;
                    }
                    else
                    {
                        fmprintf(&System->Out, "not a directory\n");
                    }
                }
                else if (strcmp(pArgv, "dis") == 0)
                {
                    pArgv += strlen(pArgv) + 1; // move to the next argument
                    File_t *Executable = Directory_FindFileByRelativePath(CurrentWorkingDir, pArgv);
                    if (Executable)
                    {
                        DisassembleExecutable(&Executable->Mem, &System->Out);
                    }
                    else
                    {
                        fmprintf(&System->Out, "not a file\n");
                    }
                }
                else
                {
                    fmprintf(&System->Out, "too many arguments\n");
                }
            }
        }

        Sys_FlushOut(System, NULL, NULL);
    }

    Directory_FreeRecursive(&System->RootDir);

    return EXIT_SUCCESS;
}

void FlushFn(Byte *Data, size_t Length)
{
    fwrite(Data,
           1,
           Length,
           stdout);
}

int main(int argc, const char **argv)
{
    (void)argv;

    if (argc < 1)
    {
        printf("no program name\n");
        return EXIT_FAILURE;
    }

    System_t System;
    Kernel_Init(&System);
    System.Config.FlushCallback = FlushFn;

    Byte ExitCode = 0;
    if (argc > 1)
    {
        ExitCode = ExecuteNonvirtualFile(argv[1], &System);
    }
    else
    {
        ExitCode = REPL(&System);
    }

    Kernel_Free(&System);

    return ExitCode;
}
