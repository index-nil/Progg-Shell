




#ifndef TYPES_H
#define TYPES_H
#include <stdbool.h>
#include <stddef.h>
#include <ctype.h>


typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} DynamicBuffer;

typedef struct {
    bool noFlag;
    bool exitAfter;
    bool shutDownAfter;
    bool debugMode;
} PSH_GlobalFlags;

typedef struct {
    char *data;
    size_t capacity;
} ShellMemory;
typedef struct List {
    char* data;           
    struct List* next;
} List;
typedef struct {
    int Readtime; //User Read Time
    char* Lng; //User selected language file
} userinf;

/*
typedef struct StringVariable {
    char* name;
    char* value;
    struct StringVariable* next;
} StringVariable;
typedef struct NumberVariable {
    char* name;
    long value;
    struct NumberVariable* next;
} NumberVariable;*/


#endif
