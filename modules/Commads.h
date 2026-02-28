#ifndef MAIN_COMMANDS_H
#define MAIN_COMMANDS_H
#include <stdbool.h>
#include "types.h"





/*---------------Functions------------------*/

    void strToLower(char *str);
    void shutdownComputer();
    int MoveFileFunct (char* filepatch, char* newpath); //Need Be deleted
    void create(char* cmd);
    int cpuload();
    int setlng(char * CmdSource);
    void ver();
    void CmdOpen();
    void help();
    void memload();
    void cls();
    void delay(char* cmd);
    void crt(char* CmdSource, ShellMemory* mem);
    void run(char* CmdSource);
    //void stringVarCreateCommand(char* CmdSource, StringVariable* vars);
    int mov(char* CmdSource, ShellMemory* mem) ;
    void ifFunct(char* CmdSource);
    bool copy(char* CmdSource, ShellMemory* mem) ;
    int cd(char* CmdSource, ShellMemory* mem) ;
    void mdr(char* CmdSource, ShellMemory* mem) ;
    void dir(ShellMemory* mem);
    void ren(char * CmdSource, ShellMemory* mem);
    void del(char* CmdSource, ShellMemory* mem);
    void prt(char* CmdSource);
    void prtread(char* CmdSource);
    void execCommand(char* CmdSource, PSH_GlobalFlags *flags,ShellMemory* mem); //StringVariable* vars
    void NeoOpen(char* CmdSource, ShellMemory* mem);
    void init_start_path(ShellMemory* mem);
    void psh_change_dir(const char* target, ShellMemory* mem);
    userinf OpenUserinf();
    char* ReadUserinfLanguage();
    int ReadUserinfReadtime();
    void ReadTime();
    int export_file(char* CmdSource,ShellMemory* mem);



#endif