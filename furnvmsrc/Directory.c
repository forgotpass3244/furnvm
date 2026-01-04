
#include <string.h>
#include "Directory.h"
#include "File.h"

FileOrDir_t Directory_FindFileOrDirByRelativePath(Directory_t *Dir, const char *Path)
{
    FileOrDir_t Current = {0};

    Current.IsDir = true;
    Current.As.Dir = Directory_GetRootOf(Dir); // start at the root unless "./"

    char Buffer[56] = {0};
    size_t BufPosition = 0;
    size_t Length = strlen(Path);
    for (size_t i = 0; i < Length; i++)
    {
        if (Current.As.Dir == NULL) // applies to file as well since union
        {
            return (FileOrDir_t) {0};
        }


        char c = Path[i];

        if (c == '.')
        {
            if (Path[i + 1] == '.')
            {
                ++i;
                // to parent directory
                if (Current.As.Dir->Parent)
                {
                    Current.As.Dir = Current.As.Dir->Parent;
                }
            }
            else
            {
                // go to the original directory
                Current.IsDir = true;
                Current.As.Dir = Dir;
            }

            // clear buffer
            BufPosition = 0;
            for (size_t j = 0; j < sizeof(Buffer); j++)
            {
                Buffer[j] = 0;
            }
        }
        else

            if (c == '/')
        {
            if (BufPosition > 0)
            {
                Current = Directory_Get(Current.As.Dir, Buffer);
            }

            // clear buffer
            BufPosition = 0;
            for (size_t i = 0; i < sizeof(Buffer); i++)
            {
                Buffer[i] = 0;
            }
        }
        else
        {
            Buffer[BufPosition++] = c;
        }
    }

    if (BufPosition > 0)
    {
        Current = Directory_Get(Current.As.Dir, Buffer);
    }

    return Current;
}

File_t *Directory_FindFileByRelativePath(Directory_t *Dir, const char *Path)
{
    FileOrDir_t Current = Directory_FindFileOrDirByRelativePath(Dir, Path);

    if (Current.As.Dir == NULL) // its a union
    {
        return NULL;
    }
    else if (Current.IsDir)
    {
        return NULL; // not a file
    }
    else
    {
        return Current.As.File;
    }
}

Directory_t *Directory_FindDirByRelativePath(Directory_t *Dir, const char *Path)
{
    FileOrDir_t Current = Directory_FindFileOrDirByRelativePath(Dir, Path);

    if (Current.As.Dir == NULL) // its a union
    {
        return NULL;
    }
    else if (!Current.IsDir)
    {
        return NULL; // not a directory
    }
    else
    {
        return Current.As.Dir;
    }
}

FileOrDir_t Directory_Get(Directory_t *Dir, const char *Name)
{
    {
        LsNode *Node = Dir->Ls;
        while (Node)
        {
            if (strcmp(Node->Name, Name) == 0)
            {
                Node->File->Position = 0;
                return (FileOrDir_t) { .IsDir = false, .As.File = Node->File };
            }

            Node = Node->Next;
        }
    }

    // check for subdirectories
    {
        SubdirNode *Node = Dir->Subdirs;
        while (Node)
        {
            if (strcmp(Node->Name, Name) == 0)
            {
                return (FileOrDir_t) {.IsDir = true, .As.Dir = Node->Dir};
            }

            Node = Node->Next;
        }
    }

    return (FileOrDir_t) {0};
}

Directory_t *Directory_GetRootOf(Directory_t *Dir)
{
    if (Dir == NULL)
    {
        return NULL;
    }

    Directory_t *Current = Dir;
    while (Current->Parent)
    {
        Current = Current->Parent;
    }

    return Current;
}

void LsNode_Append(LsNode *List, LsNode *NewNode)
{
    LsNode *Node = List;
    while (Node->Next)
    {
        Node = Node->Next;
    }
    Node->Next = NewNode;
}

void Directory_AddFile(Directory_t *Dir, File_t *File, char *Name)
{
    LsNode *Node = malloc(sizeof(LsNode));
    Node->File = File;
    Node->Name = Name;
    Node->Next = NULL;

    if (Dir->Ls == NULL)
    {
        Dir->Ls = Node;
    }
    else
    {
        LsNode_Append(Dir->Ls, Node);
    }
}

void SubdirNode_Append(SubdirNode *List, SubdirNode *NewNode)
{
    SubdirNode *Node = List;
    while (Node->Next)
    {
        Node = Node->Next;
    }
    Node->Next = NewNode;
}

void Directory_AddSubdir(Directory_t *Dir, Directory_t *Subdir, char *Name)
{
    Subdir->Parent = Dir;
    SubdirNode *Node = malloc(sizeof(SubdirNode));
    Node->Dir = Subdir;
    Node->Name = Name;
    Node->Next = NULL;

    if (Dir->Subdirs == NULL)
    {
        Dir->Subdirs = Node;
    }
    else
    {
        SubdirNode_Append(Dir->Subdirs, Node);
    }
}

void Directory_FreeRecursive(Directory_t *Dir)
{
    {
        LsNode *Node = Dir->Ls;
        while (Node)
        {
            LsNode *OldNode = Node;
            Node = OldNode->Next;
            free(OldNode->Name);
            free(OldNode->File);
            free(OldNode);
        }
    }

    {
        SubdirNode *Node = Dir->Subdirs;
        while (Node)
        {
            SubdirNode *OldNode = Node;
            Node = OldNode->Next;
            Directory_FreeRecursive(OldNode->Dir);
            free(OldNode);
        }
    }

    if (Dir->Parent) // roots are on the stack
    {
        free(Dir);
    }
}
