

#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>
#include <ctype.h>





typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} DynamicBuffer;



typedef struct {
    char *data;
    size_t capacity;
} ShellMemory;

typedef struct {
    int Readtime; //User Read Time
    char* Lng; //User selected language file
    int MaxCommandsInHistory; // Max commands in History file
    // int SortType; // User sort type 1 - folders first 2 - files first
} userinf;

typedef struct {
    int CommandsCountToExecute;
    ShellMemory* Path;
    char* Command;
} Future;

#ifdef _WIN32

typedef struct {
    char  *ext;
    int    count;
} ExtEntry;

typedef struct {
    ExtEntry *data;
    size_t    size;
    size_t    capacity;
} ExtMap;

#endif

typedef struct { //Commands imported from PATH
    
    char* Name;
    int PATH_Index;

    
} Command;

typedef struct {

    char* name;
    char* data;

} Variable;




#endif
