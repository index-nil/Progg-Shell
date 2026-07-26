

/*----------------------------------------------------------------------*/

#define _DEFAULT_SOURCE
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include "print.h"
#include "fum.h"
#include "types.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../main.h"
#ifdef _WIN32

    #define osPathSeparateSymbol ';'
    #define osPathSeparateString ";"
    #define osPathSeparateSlash '\\'
    #include <io.h>
#else
    #define osPathSeparateSymbol ':'
    #define osPathSeparateString ":"
    #define osPathSeparateSlash '/'

    #include <unistd.h>
#endif




char * Get_Name_From_Path(const char* Path){ //Get name of file from path. Returns pointer to allocated heap memory. DONT FORGOT FREE()
    
    
    size_t len = 0;
    
    while (Path[len] != '\0'){
        len++;
        
    }
    
    if (len < 1){
        
        return NULL;
    }
    
    
    
    size_t i = 1; //negative index
    while (Path[len - i] != '/'){ 
        
        i++;
        
    }
    
    i--;
    
    char* returnVal = malloc(i + 1);
    
    if (returnVal == NULL){
        return NULL;
    }
    
    for (size_t a = 0; a < i; a++){
        returnVal[a] = Path[len - i + a];
        #ifdef _WIN32
        if (returnVal[a + 1] == '.'){ //Dont add file ext to name 
            break;
        }
        
        #endif
    }
    
    returnVal[i] = '\0';
    
    return returnVal;
    
}





int Find_In_PATHS_Massive(const char* path, const char* Massive){ //Searching in PATH massive str and return index (-1 on error)

    int index = 0;
    size_t i = 0;

    unsigned int inPathIndex = 0;
    unsigned int inPath = 0;

    while(Massive[i] != '\0'){

        if(Massive[i] == path[inPathIndex]){
            inPath = 1;
            inPathIndex++;

            if(path[inPathIndex] == '\0' &&
               (Massive[i + 1] == '\0' || Massive[i + 1] == osPathSeparateSymbol)){
                return index;
            }
        }
        else{
            inPath = 0;
            inPathIndex = 0;
            while(Massive[i] != osPathSeparateSymbol && Massive[i] != '\0'){
                i++;
            }
            if(Massive[i] == osPathSeparateSymbol){
                index++;
                i++;
            }
            continue;
        }

        i++;
    }

    return -1;
}

// char* Get_Command_Path_From_PATHS_Massive(char* Command_Name, char* Massive, unsigned int C_Count, Command* Commands_Massive){

//     if (Command_Name = NULL || Massive == NULL){
//         return NULL;
//     }

//     Command* entry = NULL;

//     for (size_t i = 0; i < C_Count; i++){
        
//         if (strcmp(Command_Name, Commands_Massive[i].Name) == 0){
//             entry = &Commands_Massive[i];
//             break;
//         }

//     }
    
//     size_t a = 0;

//     if (entry == NULL){
//         return NULL;
//     }
    
//     for (size_t i = 0; i < entry->PATH_Index; i++){

//         while (Massive[a] != '\0' && Massive[a] != osPathSeparateSymbol){
//             a++;
//         }
//     }

//     a++;

//     char* buf = malloc(strlen(Massive) + 1);

//     if (buf == NULL){
//         return NULL;
//     }


//     while (Massive[a] != '\0' && Massive[a]!= osPathSeparateSymbol){
//         buf[a] = Massive[a];
//     }

//     Massive[a + 1]  = '\0';
    
    
//     return buf;
    
    

// }
 

int AddPATHCommandFromDir(const char* PATH_To_dir, char** PATH_Massive_P, Command** CommandList, unsigned int* C_Count){ 

    
    if (access(PATH_To_dir,R_OK) == 0){
        
        if (is_directory(PATH_To_dir) == 1){

            DIR* dr = opendir(PATH_To_dir);
            struct dirent *entry;



            while ((entry = readdir(dr))){

                struct stat st;
                char* Full_Path = CombinePath(PATH_To_dir, entry->d_name);
                stat(Full_Path, &st);
                free(Full_Path);

                if (S_ISDIR(st.st_mode) && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0){
                    char* Full_Path_To_Dir = CombinePath(PATH_To_dir, entry->d_name);
                    AddPATHCommandFromDir(Full_Path_To_Dir,PATH_Massive_P,CommandList,C_Count);
                    free(Full_Path_To_Dir);
                }
                #ifndef _WIN32
                else if (entry->d_type == DT_LNK) {
                    struct stat st;
                    char* full = CombinePath(PATH_To_dir, entry->d_name);
                    if (stat(full, &st) == 0) {
                        if (S_ISDIR(st.st_mode)) {
                            AddPATHCommandFromDir(full, PATH_Massive_P, CommandList, C_Count);
                        } else if (S_ISREG(st.st_mode)) {
                                               *CommandList = realloc(*CommandList, (*C_Count + 1) * sizeof(Command));
                            if (*CommandList == NULL){
                                error("Memory allocation error!");
                                closedir(dr);
                                return -5;
                            }
                            
                            
                            (*CommandList)[*C_Count].Name = strdup(entry->d_name);
                            //printf("Command: %s | Path_Massive: %s\n",(*CommandList)[*C_Count].Name, *PATH_Massive_P);

                            
                            if ((*CommandList)[*C_Count].Name == NULL){
                                closedir(dr);
                                return -5;
                            }
                            
                            
                            
                            int PATH_Index = Find_In_PATHS_Massive(PATH_To_dir,*PATH_Massive_P); //Place holder
                            
                            
                            if (PATH_Index > -1){
                                (*CommandList)[*C_Count].PATH_Index = PATH_Index;
                                (*C_Count)++;
                            }
                            else{
                                char* tmp = realloc(*PATH_Massive_P, strlen(*PATH_Massive_P) + strlen(PATH_To_dir) + 2);
                                
                                if (tmp == NULL){
                                    free((*CommandList)[*C_Count].Name);
                                    error("Memory allocation error!");
                                    closedir(dr);
                                    return -5;
                                }
                                
                                *PATH_Massive_P = tmp;
                                

                                char* prep = CombineStrings(osPathSeparateString, PATH_To_dir);
                                strcat(*PATH_Massive_P, prep);
                                free(prep);

                                
                                PATH_Index = Find_In_PATHS_Massive(PATH_To_dir,*PATH_Massive_P);
                                (*CommandList)[*C_Count].PATH_Index = PATH_Index;
                                (*C_Count)++;
                                
                            }
                        }
                    }
                    free(full);
                }
                #endif
                else if (S_ISREG(st.st_mode)){
                    
                    *CommandList = realloc(*CommandList, (*C_Count + 1) * sizeof(Command));
                    if (*CommandList == NULL){
                        error("Memory allocation error!");
                        closedir(dr);
                        return -5;
                    }
                    
                    
                    (*CommandList)[*C_Count].Name = strdup(entry->d_name);
                    //printf("Command: %s | Path_Massive: %s\n",(*CommandList)[*C_Count].Name, *PATH_Massive_P);

                    
                    if ((*CommandList)[*C_Count].Name == NULL){
                        closedir(dr);
                        return -5;
                    }
                    
                    
                    
                    int PATH_Index = Find_In_PATHS_Massive(PATH_To_dir,*PATH_Massive_P);
                    
                    
                    if (PATH_Index > -1){
                        (*CommandList)[*C_Count].PATH_Index = PATH_Index;
                        (*C_Count)++;
                    }
                    else{
                        char* tmp = realloc(*PATH_Massive_P, strlen(*PATH_Massive_P) + strlen(PATH_To_dir) + 2);
                        
                        if (tmp == NULL){
                            free((*CommandList)[*C_Count].Name);
                            error("Memory allocation error!");
                            closedir(dr);
                            return -5;
                        }
                        
                        *PATH_Massive_P = tmp;
                        


                        char* prep = CombineStrings(osPathSeparateString, PATH_To_dir);
                        strcat(*PATH_Massive_P, prep);
                        free(prep);

                        
                        PATH_Index = Find_In_PATHS_Massive(PATH_To_dir,*PATH_Massive_P);
                        (*CommandList)[*C_Count].PATH_Index = PATH_Index;
                        (*C_Count)++;
                        
                    }
                }
                else if(strncmp(entry->d_name, ".",1) != 0){
                    print_s("\033[38;2;209;0;0m[Warning]: Unknow type of '%s'\033[0m\n", entry->d_name);
                }
                    
                    
                
                
            }
            
            closedir(dr);
            return 0;
        }
    }

    return 0;
}







int AddPATHCommandstoCommandsCompletion(Command ** Commands_Massive_P, char ** PATHS_Massive_P, unsigned int* Commands_Count){ // Add executable from PATH to Commands completion (Need be optimizate RAM)
    
    

    
        
    char *path = getenv("PATH"); //Get PATH env from the system
    
    if (path == NULL){
        error("Failed to get PATH env cant add commands from PATH!");
        return -2;
    }
        
    
    
    size_t pathLen = 0; // Init and remove garbage from variable
    unsigned int pathsToFolders = 0;
    size_t i = 0; // Just index var

    unsigned int C_Count = 0;
    
    while (path[pathLen] != '\0'){
        
        if (path[pathLen] == osPathSeparateSymbol){
            pathsToFolders++;
        }

        pathLen++;
        
        
    }

    #ifdef _WIN32
    const char * defaults[] = { //Placeholder
            "hello",
            "cls",
            "cd",
            "ver",
            "mdr",
            "crt",
            "cop",
            "mov",
            "fl",
            "ren",
            "clv",
            "q",
            "quit",
            "exit",
            "create",
            "del",
            "prtread",
            "prt",
            "run",
            "delay",
            "set-lng",
            "help",
            "if",
            "histexport",
            "clh",
            "back",
            "future",
            "find",
            "nocommands",
            "excommands",
            "varlist",
            "findf",
            "inf",
            NULL
        };
        #else
        const char * defaults[] = { //Placeholder
                "hello",
                "cd",
                "ver",
                "clv",
                "q",
                "quit",
                "exit",
                "delay",
                "set-lng",
                "help",
                "if",
                "histexport",
                "clh",
                "back",
                "future",
                "nocommands",
                "excommands",
                "varlist",
                NULL
            };
        #endif




    unsigned int currentPathLen = 0;
    unsigned int errors_count = 0;
    int a = 0;
    int start = 0;

    int PATH_Index = 0;
    Command * CommandList = malloc(pathsToFolders * sizeof(Command));
    char* FoldPath = malloc(pathLen + 1); // Full folder path buf 
    
    

    if (CommandList == NULL){

        if (FoldPath != NULL){
            free(FoldPath);
        }
        error("Memory allocation error!");
        return -3;

    }
    
    if (FoldPath == NULL){
        if (CommandList != NULL){
            free(CommandList);
        }
        error("Memory allocation error!");
        return -4;
    }
    
    
    *PATHS_Massive_P = strdup(path);

    i = 0;

    while (defaults[C_Count] != NULL){

        CommandList = realloc(CommandList, (C_Count + 1) * sizeof(Command));

        if (CommandList == NULL){
            error("Memory allocation error!");
            free(FoldPath);
            free(*PATHS_Massive_P);
            free(CommandList);
            return -5;
        }
        
        CommandList[C_Count].Name = strdup(defaults[C_Count]);
        
        if (CommandList[C_Count].Name == NULL){
            free(FoldPath);
            free(CommandList);
            free(*PATHS_Massive_P);
            return -5;
        }
        
        
        
        
        CommandList[C_Count].PATH_Index = -1;

        C_Count++;
        
    }
    
    

    for (i = 0; i < pathsToFolders; i++){
       
        start = a;
        while (path[a] != '\0' && path[a] != osPathSeparateSymbol){
            
            FoldPath[a - start] = path[a];
            currentPathLen++;
            a++;
            
        }
        
        FoldPath[a - start] = '\0';
        
        if (path[a] == osPathSeparateSymbol) a++;

        if (access(FoldPath,R_OK) == 0){
            
            if (is_directory(FoldPath) == 1){
                AddPATHCommandFromDir(FoldPath,PATHS_Massive_P,&CommandList,&C_Count);
            }



        }
        else{
            print_s("\033[38;2;209;0;0m[Error]: Cant get read access to '%s'\033[0m\n", FoldPath);

            errors_count++;

            if (errors_count >= pathsToFolders){

                error("Cant get read access to folders in PATH env!");
                free(*PATHS_Massive_P);
                free(FoldPath);
                free(CommandList);
                return -6;

            }
            

            continue;
        }

        

    }
    
    *Commands_Massive_P = CommandList;
    *Commands_Count = C_Count;
    free(FoldPath);
    
    return 0;
    
    
}
