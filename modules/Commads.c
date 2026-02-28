
#include <time.h>
#include "Commads.h"
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include "types.h"
#include "../main.h"
#include "Sys_load.h"
#include "Neopad.h"
#include "fum.h" //File Utils Module import 
#include "UIlib.h" //Progress bar, graph and other
#include "LngModule.h" //Localization
#include "Neopad.h" //Text editor
#include "cJSON.h" //Json parser for user-inf.json
#include <string.h> //String functions
#include <stdbool.h> //Bool variables
#include <ctype.h>//Lower command library
#include <stdlib.h>  // System control
#include <errno.h> //Error number print 
#include <readline/readline.h>
#include <readline/history.h>
#include "print.h"
// #include "modules/ExtTools.h" //Extension Tools
#include <dirent.h> // dir || fl commands
// #define NCURSES_STATIC
#include <stddef.h>
#define Settings "user-inf.json"

#ifdef _WIN32
    #define OS_WINDOWS
    #include <ncurses/ncurses.h>
    #include <windows.h>
    #include <direct.h>
    #define GetCurrentDir _getcwd
    #define ChangeDir _chdir
    #define WIN32_LEAN_AND_MEAN
    #define SLEEP_MS(x) Sleep(x)
#else
    #include <ncurses.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
    #define GetCurrentDir getcwd
    #define ChangeDir chdir
    #define SLEEP_MS(x) usleep((x)*1000)
    #define OS_LINUX
#endif
/*------------Variable create------------*/


CpuMonitor monitor;
float cores_load[MAX_CORES];


volatile sig_atomic_t Exit = false;




void strToLower(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}
void shutdownComputer() {
    printf("Shutting down computer...");
    #ifdef OS_WINDOWS
    if (!system("shutdown /s /t 0")){(void)_write(1,"Failed to shutdown computer! Please try again\n",46);};      
    // system("shutdown /r /t 0");   
    #elif defined(OS_LINUX)
    if (!system("shutdown -h now")){(void)write(1,"Failed to shutdown computer! Please try again\n",46);};       
    // system("reboot");             
    #else
    write(1,"Unknow System!\n",15);
    #endif
    exit(0);
}
void Stop(int sig){
    (void)sig;
    Exit = !Exit;
}

int MoveFileFunct (char* filepatch, char* newpath){ //Need Be deleted
    if (filepatch == NULL || newpath == NULL) {
        printf("Invalid file path(s) provided.\n");
        return 1;
    }
    if (rename(filepatch, newpath) == 0) {
        printf("File moved successfully.\n");
        return 0;
    } else {
        printf("Error moving file: %s\n", strerror(errno));
        return 1;
    }
    
}
void create(char* cmd){
    char *arg = cmd + 6;
    
    while (*arg == ' ') arg++;
    
    if (*arg == '\0'){
        printf("Usage: create <object_name>\n");
    }
    
    else {
        if (strcmp(arg,"progressbar")== 0){
            for (int i = 0; i < 101; i++)
            {
                
                if (i == 100){
                    create_pb("Test", 1,i,10,"|","·","|",2,169,0,184);
                    break;
                }
                create_pb("Test", 1,i,10,"|","·","|",1,169,0,184);
                SLEEP_MS(10);
            }
            
        }
        else if (strcmp(arg, "graph") == 0) {
            printf("\033[?1049h");
            for (size_t i = 0; i < 100; i++)
            {
                CreateGraph(20,i, 1, 0, "|", "└","┌", "#", "·" ,0,0,95,0);
                SLEEP_MS(100);
                printf("\033[H");
                printf("\033[K");
                
            }
            printf("\033[?1049l");
        }
    }
     
}
int cpuload(){
    printf("\033[?1049h");
    
    signal(SIGINT, Stop);
    cpu_monitor_init(&monitor);
    while (Exit != true) 
    {
        
        SLEEP_MS(1000);
        int count = cpu_monitor_update(&monitor, cores_load);
        printf("\033[K");
        printf("\033[H");
        
        printf("(^C To Exit)\n");
        printf("[CPU Load]:\n\n");
        for (int i = 0; i < count; i++) {
            char core_label[32];
                sprintf(core_label, "[Core %2d]", i); 
                
                int val = (int)cores_load[i];
                if (val > 100) val = 100;
                int r = 41;
                int g = 204; 
                int b = 0;  // Green
                if (val > 50){
                    r = 255;
                    g = 179;
                    b = 0; // Yellow
                } 
                if (val > 80){
                    r = 184;
                    g = 18;
                    b = 0; // Red
                }
                create_pb(
                    core_label, 
                    1,         
                    val,        
                    20,        
                    "#",     
                    "-",
                    "|",  
                    0,         
                    r,
                    g,
                    b      
                );
                
        }
    }
    fflush(stdout);
    Exit = false;
    printf("\033[?1049l");
    return 0;
}
int setlng(char * CmdSource) { // set language file and write
        char *arg = CmdSource + 7;
        while (*arg == ' ') arg++;
        if (strlen(arg) == 0) {
            error("Specify language file!");
            return -1;
        } else {
            char* tempLang = CombinePath(ProgrammDir.data, arg); 
            
            if (access(tempLang,0)!= 0){
                
                free(tempLang);
                
            }
            else{
                FILE *fl = fopen(Settings,"r");
                if (!fl){
                    free(tempLang);
                    return -1;
                }
                char*data;
                fseek(fl, 0, SEEK_END);
                long length = ftell(fl);
                fseek(fl, 0, SEEK_SET);
                data = (char*)malloc(length + 1);
                if (data){
                    fread(data, 1, length, fl);
                    data[length] = '\0';
                }   
                fclose(fl);
                cJSON *Parsed = cJSON_Parse(data);
                free(data);
                cJSON* langsect = cJSON_GetObjectItem(Parsed,"Language");
                cJSON_SetValuestring(langsect,tempLang);
                char *rendered = cJSON_Print(Parsed);
                FILE *fw = fopen(Settings,"w");
                int succefull = 0; //0 - true, -1 - false
                if (fw) {
                    succefull = 0;
                    fputs(rendered, fw);
                    fclose(fw);
                    printf("Language file installed to: %s\n", tempLang);
                } else {
                    succefull = -1;
                    error("Could not save settings file!");
                }


                cJSON_Delete(Parsed);
                free(tempLang); 
                return succefull;
            }
        }
}
void ver(){
    printf("\n⠀⠀⣠⡿⠉⠉⢻⣇⠀⠀⠀⠀⠀⣴⠟⠉⠉⢿⡆⠀⠀⠀⠀⢠⡾⠋⠉⠹⣷⠀\n⠀⢠⣿⠀⠀⠀⢸⡿⠀⠀⠀⠀⢸⡏⠀⠀⠀⣸⡇⠀⠀⠀⠀⣾⡇⠀⠀⢠⣿⠀\n⠀⢸⡇⠀⠀⠀⣼⡇⠀⠀⠀⠀⣿⠃⠀⠀⠀⣿⠁⠀⠀⠀⠀⠘⠻⠶⠚⣻⡇⠀\n⠀⢸⣇⠀⠀⣰⡟⠀⢀⣀⠀⠀⣿⡀⠀⢀⣼⠏⠀⣀⡀⠀⠀⠀⠀⢀⣰⡟⠀⠀\n⠀⠈⠛⠛⠛⠋⠀⠀⠘⠋⠀⠀⠈⠛⠛⠛⠁⠀⠀⠛⠃⠀⠘⠛⠛⠛⠉⠀⠀⠀\n");
    printf("\033[38;2;160;0;209mProggShell\033[0m v0.0.9\n");
    fflush(stdout);
    return;
}
void CmdOpen(){
    int Status;
    #ifdef OS_LINUX
    Status = system("bash");
    #else
    Status = system("cmd");
    #endif //MacOS?
    if (Status != 0) {
        
        printf("\033[38;2;209;0;0mError! Error code: %d\033[0m\n", Status );
    }
    else {
        printf("Returned to ProggShell\n");
    } 
}
void help(){
    char* Temp = getSection("help", langFile);
    if (Temp == NULL || !Temp){
        printf("\033[38;2;209;0;0mError:The partition of language is free or does not exist\033[0m\n");
    }
    else{
        printf("%s",Temp);
        free(Temp);
    }
}
void memload(){
        printf("\033[?1049h");
        
        signal(SIGINT, Stop);
        while (Exit != true) 
        {
            SLEEP_MS(1000);
            MemoryData memload = get_memory_stats();
            unsigned long long total_ram_mb = memload.total / 1024 / 1024;
            unsigned long long used_ram_mb = (memload.total - memload.free) / 1024 / 1024;
            

            unsigned long long total_swap_mb = 0;
            unsigned long long used_swap_mb = 0;

            if (memload.swap > 0) {
                total_swap_mb = memload.swap / 1024 / 1024;
                
                if (memload.swap >= memload.freeswap) {
                    used_swap_mb = (memload.swap - memload.freeswap) / 1024 / 1024;
                }
            }

            
            printf("\033[H");
            
            printf("(^C To Exit)\n\n");
            printf("[Memory]:%llu MB\n\n",total_ram_mb);
            printf("[SWAP]:%llu MB\n\n",total_swap_mb);
            if (used_ram_mb > (total_ram_mb / 100 * 80)){
                create_pb("[Memory Usage]:",1, (int)((used_ram_mb * 100) / total_ram_mb),20,"|","-","|",1,209,0,0);
            }
            else if (used_ram_mb > (total_ram_mb / 100 * 50)){
                create_pb("[Memory Usage]:",1, (int)((used_ram_mb * 100) / total_ram_mb),20,"|","-","|",1,255,208,0);
            }
            else if (used_ram_mb < (total_ram_mb / 100 * 50)){
                create_pb("[Memory Usage]:",1, (int)((used_ram_mb * 100) / total_ram_mb),20,"|","-","|",1,17,0,171);
            }
            printf("   %llu MB\n\n",used_ram_mb);
            
            
            if (used_swap_mb > (total_swap_mb / 100 * 80)){
                create_pb("[SWAP Usage]:  ",1, (int)((used_swap_mb * 100) / total_swap_mb),20,"|","-","|",1,209,0,0);
            }
            else if (used_swap_mb > (total_swap_mb / 100 * 50)){
                create_pb("[SWAP Usage]:  ",1, (int)((used_swap_mb * 100) / total_swap_mb),20,"|","-","|",1,255,208,0);
            }
            else if (used_swap_mb < (total_swap_mb / 100 * 50)){
                create_pb("[SWAP Usage]:  ",1, (int)((used_swap_mb * 100) / total_swap_mb),20,"|","-","|",1,17,0,171);
            }
            printf("    %llu MB\n\n",used_swap_mb);
        }
        fflush(stdout);
        Exit = false;
        printf("\033[?1049l");
}
void cls(){
    #ifdef _WIN32
        system("cls");
    #else 
        system("clear");
    #endif
}
/*
void stringVarCreateCommand(char* CmdSource, StringVariable* vars){
    char* arg1 = CmdSource + 17;
    while (*arg1 == ' ') arg1++;
    char* arg2 = arg1 + 1;
    while (*arg2 != ' ' && *arg2 != '\0') arg2++;
    add_string_var(vars,arg1,arg2);
}*/
int mov(char* CmdSource, ShellMemory* mem) {
    char *arg = CmdSource + 4;
    while (*arg == ' ') arg++;
    if (*arg == '\0') {
        printf("Usage: mov <source> <destination>\n");
        return 1; // Возвращаем код ошибки
    }

    // Поиск разделителя между аргументами
    char *arg2 = arg;
    while (*arg2 != ' ' && *arg2 != '\0') arg2++;
    if (*arg2 == '\0') {
        printf("Usage: mov <source> <destination>\n");
        return 1;
    }

    *arg2 = '\0';
    char *new_name = arg2 + 1;
    while (*new_name == ' ') new_name++;
    if (*new_name == '\0') {
        printf("Usage: mov <source> <destination>\n");
        return 1;
    }

    // Формирование пути источника
    char *old_path = NULL;
    if (arg[0] == '/' || arg[0] == '\\' || (isalpha((unsigned char)arg[0]) && arg[1] == ':')) {
        old_path = strdup(arg);
    } else {
        old_path = CombinePath(mem->data, arg);
    }

    if (!old_path) { 
        error("Memory allocation error\n"); 
        return 1; 
    }

    // Извлечение имени файла для случая, если цель — папка
    const char *src_basename = strrchr(arg, '/');
    const char *bslash = strrchr(arg, '\\');
    if (bslash && (!src_basename || bslash > src_basename)) src_basename = bslash;
    if (src_basename) src_basename++; else src_basename = arg;

    char *dst_path = NULL;
    // Обработка пути назначения
    if (new_name[0] == '/' || new_name[0] == '\\' || (isalpha((unsigned char)new_name[0]) && new_name[1] == ':')) {
        if (is_directory(new_name)) {
            size_t s = strlen(new_name) + strlen(src_basename) + 2;
            dst_path = malloc(s);
            if (dst_path) snprintf(dst_path, s, "%s/%s", new_name, src_basename);
        } else {
            dst_path = strdup(new_name);
        }
    } else {
        char *candidate = CombinePath(mem->data, new_name);
        if (!candidate) { 
            free(old_path); 
            error("Memory allocation error\n"); 
            return 1; 
        }
        if (is_directory(candidate)) {
            size_t s = strlen(candidate) + strlen(src_basename) + 2;
            dst_path = malloc(s);
            if (dst_path) snprintf(dst_path, s, "%s/%s", candidate, src_basename);
            free(candidate);
        } else {
            dst_path = candidate;
        }
    }

    if (!dst_path) { 
        error("Memory allocation error\n"); 
        free(old_path); 
        return 1; 
    }

    // Выполнение перемещения
    if (MoveFileFunct(old_path, dst_path) == 0) {
        printf("\033[38;2;33;198;0mDone\033[0m\n");
    }

    free(old_path);
    free(dst_path);
    return 0;
}
void delay(char* cmd){
    char *arg = cmd + 6;
    while (*arg == ' ') arg++;
    if (*arg == '\0'){
        printf("No time to wait!\n");
    }
    
    else {
        
        long ms = atol(arg);
        SLEEP_MS(ms);
    }
}
void crt(char* CmdSource, ShellMemory* mem){
    char *arg = CmdSource + 4; 
    while (*arg == ' ') arg++; 
    char* FullPath = CombinePath(mem->data, arg);
    if (*arg != '\0') {
        FILE *file = fopen(FullPath,"w"); 
        if (file){
            fclose(file);
            printf("\033[38;2;33;198;0mDone\033[0m\n");
        }      
        
        else {
            error(strerror(errno));
        }
    }
    free(FullPath);
    return;
}
void run(char* CmdSource){
    char *arg = CmdSource + 4;
    while (*arg == ' ') arg++;
    if (*arg == '\0'){
         printf("No command to execute!\n");
    }
     
    else {
        int Status = system(arg);
        if (Status == 0) {
            printf("\033[38;2;33;198;0mDone\033[0m\n\n");
        }
        else {
        
            printf("\033[38;2;209;0;0m[Error]: executing command '%s' Error code: %d\033[0m\n\n" , arg , Status );
        }
    }
}
void ifFunct(char* CmdSource){
    char* arg1 = CmdSource + 3;
    while (*arg1 == ' ') arg1++; 
    char* end_arg1 = arg1;
    while (*end_arg1 != ' ' && *end_arg1 != '\0') end_arg1++;
    char original_char1 = *end_arg1; 
    *end_arg1 = '\0'; 
    char* arg2 = end_arg1 + 1;
    if (original_char1 != '\0') {
        while (*arg2 == ' ') arg2++;
    }
    char* end_arg2 = arg2;
    while (*end_arg2 != ' ' && *end_arg2 != '\0') end_arg2++;
    char original_char2 = *end_arg2;
    *end_arg2 = '\0'; 
    char* arg3 = end_arg2 + 1;
    if (original_char2 != '\0') {
        while (*arg3 == ' ') arg3++;
    }
    if (strcmp(arg2, "==") == 0) {
        printf("%s\n", strcmp(arg1, arg3) == 0 ? "True" : "False"); //Place Holders
    } 
    else if (strcmp(arg2, "!=") == 0) {
        printf("%s\n", strcmp(arg1, arg3) != 0 ? "True" : "False");
    } 
    else if (strcmp(arg2, ">") == 0) {
        
        printf("%s\n", atoi(arg1) > atoi(arg3) ? "True" : "False");
    } 
    else if (strcmp(arg2, "<") == 0) {
        printf("%s\n", atoi(arg1) < atoi(arg3) ? "True" : "False");
    } 
    else {
        printf("Only '==', '!=', '>', '<' operators supported\n");
    }
    
    *end_arg1 = original_char1;
    *end_arg2 = original_char2;
}
bool copy(char* CmdSource, ShellMemory* mem) {
            /* Usage: cop <source> <destination>
           Supports: cop(copy), cln (clone) */
        int cmd_len = 4;

        char *arg = CmdSource + cmd_len;
        while (*arg == ' ') arg++;

        if (*arg == '\0') {
            printf("Usage: cop <source> <destination>\n");
            free(CmdSource);
            return true;
        }

        /* split into source and destination (simple, like other commands) */
        char *sep = arg;
        while (*sep != ' ' && *sep != '\0') sep++;
        if (*sep == '\0') {
            printf("Usage: cop <source> <destination>\n");
            free(CmdSource);
            return true;
        }
        *sep = '\0';
        char *dst = sep + 1;
        while (*dst == ' ') dst++;
        if (*dst == '\0') {
            printf("Usage: cop <source> <destination>\n");
            free(CmdSource);
            return true;
        }

        char *srcPath = CombinePath(mem->data, arg);
        char *dstPath = CombinePath(mem->data, dst);

        if (strcmp(srcPath, dstPath) == 0) {
            error("Source and destination are the same!");
            free(srcPath); free(dstPath); free(CmdSource);
            return true;
        }

        FILE *in = fopen(srcPath, "rb");
        if (!in) {
            printf("\033[38;2;209;0;0mError opening '%s': %s\033[0m\n", srcPath, strerror(errno));
            free(srcPath); free(dstPath); free(CmdSource);
            return true;
        }

        FILE *out = fopen(dstPath, "wb");
        if (!out) {
            printf("\033[38;2;209;0;0mError creating '%s': %s\033[0m\n", dstPath, strerror(errno));
            fclose(in); free(srcPath); free(dstPath); free(CmdSource);
            return true;
        }
        DynamicBuffer buf;
        /* use a larger buffer and read into buf.capacity (not buf.size) */
        init_buffer(&buf,2048);
        size_t n;
        int copy_error = 0;
        while ((n = fread(buf.data, 1, buf.capacity, in)) > 0) {
            size_t written = fwrite(buf.data, 1, n, out);
            if (written != n) {
                printf("\033[38;2;209;0;0mWrite error: %s\033[0m\n", strerror(errno));
                copy_error = 1;
                break;
            }
        }
        if (ferror(in)) {
            printf("\033[38;2;209;0;0mRead error: %s\033[0m\n", strerror(errno));
            copy_error = 1;
            
        }

        fclose(in);
        fflush(out);
        fclose(out);

        if (!copy_error) {
            done();
        }

        /* free the temporary buffer */
        free_buffer(&buf);
        free(srcPath);
        free(dstPath);
        return false;
}
int cd(char* CmdSource, ShellMemory* mem) {
    char *path_arg = CmdSource + 3; 
    while (*path_arg == ' ') path_arg++; 
    
    if (*path_arg != '\0') {
        psh_change_dir(path_arg, mem); 
    }
    
    return 0;
}
void mdr(char* CmdSource, ShellMemory* mem) {
    char *arg = CmdSource + 4; 
    while (*arg == ' ') arg++; 
    char* FullPath = CombinePath(mem->data, arg);
    if (*arg != '\0') {
        #ifdef _WIN32
        if (_mkdir(FullPath) != 0){printf("\033[38;2;209;0;0mError: %s\033[0m\n",strerror(errno));}
        else{printf("\033[38;2;33;198;0mDone\033[0m\n");}
        #else
        if (mkdir(FullPath,0777) != 0){printf("Error: %s\n",strerror(errno));}
        else{printf("\033[38;2;33;198;0mDone\033[0m\n");}
        #endif
    }
    free(FullPath);
}
void dir(ShellMemory* mem){
    char *folder_name = strrchr(mem->data, '/'); 
    
    
    if (!folder_name) folder_name = strrchr(mem->data, '\\');
    
    if (folder_name) {
        
        folder_name++; 
    } else {
        
        folder_name = mem->data;
    }
    struct dirent *entry;
    DIR *dp;
    
    
    dp = opendir(mem->data);
    
    if (dp != NULL){
        int count = 0;
        
        printf("\n--------%s fill--------\n",folder_name);
        while ((entry = readdir(dp)) != NULL) {
            
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            char* full_path = CombinePath(mem->data, entry->d_name);
            count++;
            
            if (is_directory(full_path)){
                printf("[DIR] %s",entry->d_name);
                printf("\n");
                
            }
            else
            {
                continue;
            }
            free(full_path);
        }
        rewinddir(dp);
        while ((entry = readdir(dp)) != NULL) {
            
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            char* full_path = CombinePath(mem->data, entry->d_name);
            
            if (is_directory(full_path)){
                continue;
                
            }
            else
            {
                printf("[FILE] %s",entry->d_name);
                printf("\n");
            }
            free(full_path);
        }
        printf("\n---------total: %d---------\n",count);
        closedir(dp);
    }
    else {
        printf("\033[38;2;209;0;0mError opening directory: %s\033[0m\n", strerror(errno));
    }
}
void ren(char * CmdSource, ShellMemory* mem){
    char *arg = CmdSource + 3; 
    while (*arg == ' ') arg++; 
    
    char *arg2 = arg;
    while (*arg2 != ' ' && *arg2 != '\0') arg2++;
    *arg2 = '\0'; 
    
    char *new_name = arg2 + 1; 
    while (*new_name == ' ') new_name++; 
    
    if (*arg != '\0' && *new_name != '\0') {
        char* old_path = CombinePath(mem->data, arg);
        char* new_path = CombinePath(mem->data, new_name);
        
        if (rename(old_path, new_path) == 0) {
            printf("\033[38;2;33;198;0mDone\033[0m\n");
        } else {
            printf("\033[38;2;209;0;0mError: %s\033[0m\n",strerror(errno));
        }
        
        free(old_path);
        free(new_path);
    }
    else {
        printf("Usage: ren <old_name> <new_name>\n");
    }
}
void del(char* CmdSource, ShellMemory* mem){
    char *arg = CmdSource + 4;  
    while (*arg == ' ') arg++;  
    if (*arg == '\0') {
        error(strerror(errno));
    } else {
        if (yesorno("Are you sure?")== 1){
            DynamicBuffer FullPath; //Made for future
            init_buffer(&FullPath,256);
            append_to_buffer(&FullPath, mem->data, strlen(mem->data));
            
            size_t path_len = strlen(mem->data);
            if (path_len > 0 && mem->data[path_len - 1] != '/' && mem->data[path_len - 1] != '\\') {
                #ifdef OS_WINDOWS
                    append_to_buffer(&FullPath, "\\", 1);
                #else
                    append_to_buffer(&FullPath, "/", 1);
                #endif
            }
            append_to_buffer(&FullPath, arg, strlen(arg));
            if (remove(FullPath.data) != 0) {
                error(strerror(errno));
            } else {
                printf("\033[38;2;33;198;0mFile successfully deleted!\033[0m\n");
            }
            free_buffer(&FullPath);
        }
        else{
            printf("Operation Aborted\n");
        }
        return;
    }
}
void prt(char* CmdSource){
    char *arg = CmdSource + 4;
    while (*arg == ' ') arg++;
    if (*arg == '\0'){
        printf("\n");
    }
    else {
        printf("%s\n" , arg);
    }
        
}
void prtread(char* CmdSource){
    
    char *arg = CmdSource + 8;      
    while (*arg == ' ') arg++; 
    char * Localization;
    if (*arg == '\0') {
        Localization = getSection("read:fileNameError",langFile);
        if(Localization || Localization != NULL){
            printf("%s\n",Localization);
        }
    
    } else {
        long filesize = 0;
        char *content = read_file_to_buffer(arg, &filesize);
        
        if (content){
            printf("%s\n\n\n", content);
            int words = count_words(content, filesize);
            int symbols = count_symbols(content,filesize);
            Localization = getSection("read:info",langFile);
            if(Localization || Localization != NULL){
                printf(Localization, words, symbols);
                
                // printf("%f\n",((int)readtime_calc / 60.0) * words);
                free(content);
            }
        }
        else{
            Localization = getSection("general:cantOpenFile",langFile);
            if(!Localization || Localization == NULL){
                printf("%s\n",Localization);
                
            }
        }
        
      
    }
    if (Localization != NULL){
        free(Localization);
    }
}
 //StringVariable* vars
void execCommand(char* CmdSource, PSH_GlobalFlags *flags,ShellMemory* mem){
    char *arg = CmdSource + 4;
    while (*arg == ' ') arg++;
    if (*arg == '\0'){
        printf("Usage <path_to_file>\n");
    }
    
    else {
        exec(arg,flags,mem);
    }
}
void NeoOpen(char* CmdSource, ShellMemory* mem){
        char *arg = CmdSource + 4;
        while (*arg == ' ') arg++;
                    
        if (strlen(arg) == 0) {
            printf("Usage: neo <file_path>\n");
        } else {
            printf("\033[?1049h");
            
            editorInit();
            char* Path = CombinePath(mem->data, arg);
            editorOpen(Path);
            
            
            initscr(); 
            raw(); 
            keypad(stdscr, TRUE); 
            noecho();
            
            Exit = false;
            while (!Exit) {
                editorRefreshScreen();
                
                
                int c = getch();
                
                
                if (c == 3) {
                    Exit = true;
                } else {
                    
                    
                    
                    ungetch(c);
                    editorProcessKeypress();
                }
            }
            
            
            editorClose();
            
            
            #ifdef _WIN32
            HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
            SetConsoleMode(hIn, ENABLE_EXTENDED_FLAGS | ENABLE_INSERT_MODE | ENABLE_QUICK_EDIT_MODE | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
            FlushConsoleInputBuffer(hIn);
            #endif
            
            
            clearerr(stdin);
            fflush(stdin);
            
            Exit = false; 
            free(Path);
            printf("\033[?1049l");
            printf("\n--------- Neo closed---------\n");
        }
}
void init_start_path(ShellMemory* mem) {
    DynamicBuffer temp;
    

    init_buffer(&temp, 256); 

    if (GetCurrentDir(temp.data, temp.capacity) != NULL) {
        
        
        mem->capacity = strlen(temp.data) + 16;
        mem->data = (char*)malloc(mem->capacity);
        
       
        if (mem->data) {
            strcpy(mem->data, temp.data);
        }
    } else {
        
        mem->capacity = 16;
        mem->data = (char*)malloc(mem->capacity);
        if (mem->data) {
            #ifdef _WIN32
                strcpy(mem->data, "C:\\");
            #else
                strcpy(mem->data, "/");
            #endif
        }
    }
    free_buffer(&temp);
}
void psh_change_dir(const char* target, ShellMemory* mem) {
    
    if (ChangeDir(target) == 0) {
        
        DynamicBuffer temp;
        
        init_buffer(&temp, 256); 


        while (GetCurrentDir(temp.data, temp.capacity) == NULL) {
            if (errno == ERANGE) {
                
                temp.capacity += 50; //Plus 50 Bytes
                char *new_ptr = (char*)realloc(temp.data, temp.capacity);
                if (!new_ptr) {
                    perror("\033[38;2;209;0;0mMemory allocation error\033[0m\n");
                    free_buffer(&temp);
                    return;
                }
                temp.data = new_ptr;
            } else {
                
                perror("getcwd error\n");
                free_buffer(&temp);
                return;
            }
        }

        size_t new_len = strlen(temp.data);
        
        if (new_len >= mem->capacity) {
            mem->capacity = new_len + 16;
            char *new_mem_data = (char*)realloc(mem->data, mem->capacity);
            
            if (new_mem_data) {
                mem->data = new_mem_data;
            }
        }

        strcpy(mem->data, temp.data);

        free_buffer(&temp);

    } else {
        fprintf(stderr, "Error changing dir to '%s': %s\n", target, strerror(errno));
    }
}

userinf OpenUserinf(){
    userinf General = {0};
    char*data;
    FILE *file = fopen(Settings, "rb");
    if (!file){
        printf("\033[?1049l");
        error("Error opening user-inf.json file!");
        return General;
    }
    else {
        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);
        data = (char*)malloc(length + 1);
        if (data){
            fread(data, 1, length, file);
            data[length] = '\0';
        }
        fclose(file);
    }
    cJSON *parsed = cJSON_Parse(data);
    free(data);
    if (!parsed){
        printf("\033[?1049l");
        printf("Error parsing user-inf.json file!\n");
        return General;
    }
    cJSON *Readtime = cJSON_GetObjectItem(parsed, "Readtime");
    cJSON *Language = cJSON_GetObjectItem(parsed, "Language");
    if (cJSON_IsString(Language) && (Language->valuestring != NULL)) {
        General.Lng = strdup(Language->valuestring); 
    }
    if (cJSON_IsNumber(Readtime)) {
        General.Readtime = Readtime->valueint;
    }
    
    cJSON_Delete(parsed);
    return General;
}

char* ReadUserinfLanguage(){
    userinf General = OpenUserinf();
    return General.Lng;
}

//int ReadUserinfReadtime(){
//    userinf General = OpenUserinf();
//    return General.Readtime;
//}
/*
void ReadTime() {
    // 1. Входим в альтернативный буфер терминала
    printf("\033[?1049h");
    
    time_t start_t, end_t;
    double total_t;
    
    // --- Локализация ---
    char* Localization = getSection("rdtime:readTimeGet", langFile);
    char* Localization2 = getSection("rdtime:Exitphrase", langFile);

    if (Localization && Localization2) {
        printf("%s\n", Localization2);
        printf("%s\n", Localization);
        // Очищаем память сразу после вывода, чтобы не забыть
        free(Localization);
        free(Localization2);
    } else {
        printf("\033[?1049l");
        printf("Error: Localization files missing!\n");
        if (Localization) free(Localization);
        if (Localization2) free(Localization2);
        return;
    }

    // 2. Таймер (засекаем время чтения)
    signal(SIGINT, Stop); // Обработка Ctrl+C
    start_t = time(NULL);
    
    while (Exit != true) {
        SLEEP_MS(100); 
    }
    
    end_t = time(NULL);
    total_t = difftime(end_t, start_t); // Время в секундах

    // 3. Чтение JSON файла
    FILE *file = fopen(Settings, "rb");
    char *data = NULL;
    if (!file) {
        printf("\033[?1049l");
        printf("Error: Could not open user-inf.json\n");
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    data = (char*)malloc(length + 1);
    if (data) {
        fread(data, 1, length, file);
        data[length] = '\0';
    }
    fclose(file);

    if (!data) return;

    cJSON *parsed = cJSON_Parse(data);
    if (!parsed) {
        printf("\033[?1049l");
        printf("Error: Failed to parse JSON!\n");
        free(data);
        return;
    }

    // 4. Логика расчета скорости
    cJSON *Readtime_item = cJSON_GetObjectItem(parsed, "Readtime");

    double text_volume = 12000.0; // Замени на реальное кол-во знаков в тексте
    
    if (total_t > 2.0) { // Считаем, только если читали дольше 2 секунд
        int speed = (int)(text_volume / (total_t / 60.0));
        
        // Защита от неадекватных чисел (например, если пролистали мгновенно)
        if (speed > 5000) speed = 5000; 
        
        cJSON_SetNumberValue(Readtime_item, speed);
    } else {
        printf("\nToo Slow!\n");
    }

    // 5. Сохранение изменений
    char *out_str = cJSON_Print(parsed);
    if (out_str) {
        FILE *wfile = fopen(Settings, "w"); 
        if (wfile) {
            fputs(out_str, wfile);
            fclose(wfile);
        }
        free(out_str);
    }


    free(data);
    cJSON_Delete(parsed);
    
    fflush(stdout);
    Exit = false; 
    printf("\033[?1049l");
}*/
int export_file(char* CmdSource,ShellMemory* mem){
        char* arg = CmdSource + 7;
        while (*arg == ' ') arg++; 
        char* Path = CombinePath(mem->data,arg);
        if (access(Path,F_OK)== 0){
            printf("[Warning]: In this folder alreandy exist '%s' ", arg);

            if (yesorno("replace?")== 0){
                printf("Operation Aborted\n");
                return -1;
            }

            
            
        }
        if (write_history(Path) == 0) {
            printf("History exported to '%s' successfully.\n", arg);
            free(Path);
            return 0;
        } else {
            perror("Export failed\n");
            free(Path);
            return -2;
        }
}
