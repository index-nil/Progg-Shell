
#ifndef PATH_TOOLS_H
#define PATH_TOOLS_H

#include "types.h"
#include <sys/types.h>


int AddPATHCommandstoCommandsCompletion(Command ** Commands_Massive_P, char ** PATHS_Massive_P, unsigned int* Commands_Count);

char* Get_Command_Path_From_PATHS_Massive(char* Command_Name, char* Massive, unsigned int C_Count, Command* Commands_Massive);









#endif