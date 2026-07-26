#ifndef MAIN_COMMANDS_H
#define MAIN_COMMANDS_H
#include <stdbool.h>
#include "types.h"
#include <stdint.h>

extern char** psh_hist_entries;
extern int    psh_hist_count;
extern int    psh_hist_head;
extern int    MaxCommandsInHistory;


/*---------------Functions------------------*/

    #ifdef _WIN32
    void shutdownComputer();
    int MoveFileFunct (char* filepatch, char* newpath); //Need Be deleted
    void create(char* cmd);
    void cls();
    void crt(char* CmdSource, ShellMemory* mem);
    void run(char* CmdSource);
    int mov(char* CmdSource, ShellMemory* mem) ;
    int copy(char* CmdSource, ShellMemory* mem) ;
    void mdr(char* CmdSource, ShellMemory* mem) ;
    void dir(ShellMemory* mem);
    void ren(char * CmdSource, ShellMemory* mem);
    void del(char* CmdSource, ShellMemory* mem);
    void prt(char* CmdSource);
    void prtread(char* CmdSource);
    int count_files_ext(const char* Path, char* CmdSource,ExtMap * map, int isChild); // count objects extensions
    int find(char* CmdSource, ShellMemory* mem,int mode);
    void FileInfo(char* CmdSource, ShellMemory* mem);
    void solve(char* CmdSource);
    #endif
    
    void ver();
    void help();
    void clv();
    void delay(char* cmd);
    void ifFunct(char* CmdSource);
    int cd(char* CmdSource, ShellMemory* mem) ;
    void strToLower(char *str);
    int8_t setlng(const char* Path ,char * CmdSource);
    // void execCommand(char* CmdSource,ShellMemory* mem); //StringVariable* vars
    userinf OpenUserinf();
    void init_start_path(ShellMemory* mem);
    void psh_change_dir(const char* target, ShellMemory* mem);
    int8_t export_file(char* CmdSource,ShellMemory* mem);
    int back(char* CmdSource,ShellMemory* mem);
    void future(char * CmdSource, ShellMemory* mem);
    void hello();

    int create_var(char* cmd, Variable** Global_Variable_List, unsigned int * Variables_Count);
    int remove_var (char* VarName, Variable ** Global_Variable_List, unsigned int* Variables_Count);
    void List_Variables(Variable* Var_list, unsigned int Var_Count);



#endif