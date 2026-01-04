
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <string.h>
#include "File.h"

typedef struct FileOrDir_t FileOrDir_t;
typedef struct Directory_t Directory_t;

struct FileOrDir_t
{
    bool IsDir;
    union
    {
        Directory_t *Dir;
        File_t *File;
    } As;
};

typedef struct LsNode LsNode;
struct LsNode
{
    // linked list
    File_t *File;
    char *Name;
    LsNode *Next;
};

typedef struct SubdirNode SubdirNode;
struct SubdirNode
{
    // linked list
    Directory_t *Dir;
    char *Name;
    SubdirNode *Next;
};

struct Directory_t
{
    LsNode *Ls;
    Directory_t *Parent;
    SubdirNode *Subdirs;
};

FileOrDir_t Directory_FindFileOrDirByRelativePath(Directory_t *Dir, const char *Path);

File_t *Directory_FindFileByRelativePath(Directory_t *Dir, const char *Path);

Directory_t *Directory_FindDirByRelativePath(Directory_t *Dir, const char *Path);

FileOrDir_t Directory_Get(Directory_t *Dir, const char *Name);

Directory_t *Directory_GetRootOf(Directory_t *Dir);

void LsNode_Append(LsNode *List, LsNode *NewNode);

void Directory_AddFile(Directory_t *Dir, File_t *File, char *Name);

void SubdirNode_Append(SubdirNode *List, SubdirNode *NewNode);

void Directory_AddSubdir(Directory_t *Dir, Directory_t *Subdir, char *Name);

void Directory_FreeRecursive(Directory_t *Dir);

#endif // DIRECTORY_H
