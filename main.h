// main.h
#ifndef MAIN_H
#define MAIN_H
#include "modules/types.h"





int exec(char* arg,ShellMemory* mem);
void init_start_path(ShellMemory* mem);
void init_buffer(DynamicBuffer *db, size_t initial_capacity);
void free_buffer(DynamicBuffer *db);
void append_to_buffer(DynamicBuffer *db, const char *new_data, size_t data_len);
void psh_change_dir(const char* path, ShellMemory* mem);
void editorRefreshScreen();
//void add_string_var(StringVariable* vars, char* name, char* value);
extern char* langFile;
extern char* ProgrammDir;
extern Future futurecommand;
extern char* Settings;
extern int futureexecute;
extern int Readtime;
extern long* Commands_Count;

#endif