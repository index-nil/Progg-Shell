// main.h
#ifndef MAIN_H
#define MAIN_H
#include "modules/types.h"





List* push(List* head_ref, char* new_string);
int exec(char* arg,PSH_GlobalFlags* flags,ShellMemory* mem);
void freeList(List* head);
void init_start_path(ShellMemory* mem);
void init_buffer(DynamicBuffer *db, size_t initial_capacity);
void free_buffer(DynamicBuffer *db);
void append_to_buffer(DynamicBuffer *db, const char *new_data, size_t data_len);
void psh_change_dir(const char* path, ShellMemory* mem);
void editorRefreshScreen();
//void add_string_var(StringVariable* vars, char* name, char* value);
extern char* langFile;
extern DynamicBuffer ProgrammDir;


#endif