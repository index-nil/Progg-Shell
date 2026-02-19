
/*-------------------------------------Progg Shell MIT License--------------------------------------*/
#include <stdio.h> // CMD main
#include <string.h> //String functions
#include <stdbool.h> //Bool variables
#include <ctype.h> //Lower command library
#include <stdlib.h>  // System control
#include <errno.h> //Error number print 
#include <locale.h> //CMD set system language
#include <dirent.h> // dir || ls || fl commands
#include "modules/fum.h" //File Utils Module import 
#include "modules/UIlib.h" //Progress bar, radio button and other
#include "modules/LngModule.h" //Localization
#include "modules/Neopad.h" //Text editor
// #include "modules/ExtTools.h" //Extension Tools
#include "modules/Sys_load.h"
#include <time.h>
// #include "modules/mongoose.h"
#define NCURSES_STATIC
#include <ncurses/ncurses.h>
#include <signal.h>
#ifdef _WIN32
    #define OS_WINDOWS
    #include <windows.h>
    #include <direct.h>
    #define GetCurrentDir _getcwd
    #define ChangeDir _chdir
    #define WIN32_LEAN_AND_MEAN
    #define SLEEP_MS(x) Sleep(x)
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
    #define GetCurrentDir getcwd
    #define ChangeDir chdir
    #define SLEEP_MS(x) usleep((x)*1000)
    #endif
    

    
#ifdef __linux__
    #define OS_LINUX
#elif __APPLE__
#define OS_MAC
#endif

void shutdownComputer() {
    printf("Shutting down computer...");
    #ifdef OS_WINDOWS
    system("shutdown /s /t 0");      
    // system("shutdown /r /t 0");   
    #elif defined(OS_LINUX)
    system("shutdown -h now");       
    // system("reboot");             
    #elif defined(OS_MAC)
    system("shutdown -h now");       
    // system("sudo reboot");
    #else
    printf("Unknow System!\n");
    #endif
    exit(0);
}
void strToLower(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}


/*--------------Variable create---------------*/

typedef struct {
    char *data;     
    size_t size;    
    size_t capacity;
} DynamicBuffer;

typedef struct {
    bool noFlag; //{n
    bool exitAfter;   // {q
    bool shutDownAfter; //{s
    bool debugMode;   // {d
} PSH_GlobalFlags;

typedef struct {
    char *data;      
    size_t capacity; 
} ShellMemory;

// typedef struct {
//     int Readtime; //For future
// } userinf;

typedef struct Node {
    char* data;
    struct Node* next; 
} List;




CpuMonitor monitor;
float cores_load[MAX_CORES];
char* langFile;
DynamicBuffer ProgrammDir;
volatile sig_atomic_t Exit = false;


// #ifdef _WIN32
//     bool WindowsWindowCreator = false;
// #endif



// userinf OpenUserinf(){
//     userinf General = {0};
//     char*data;
//     FILE *file = fopen("user-inf.json", "rb");
//     if (!file){
//         printf("\033[?1049l");
//         printf("Error opening user-inf.json file!\n");
//         return General;
//     }
//     else {
//         fseek(file, 0, SEEK_END);
//         long length = ftell(file);
//         fseek(file, 0, SEEK_SET);
//         data = (char*)malloc(length + 1);
//         if (data){
//             fread(data, 1, length, file);
//             data[length] = '\0';
//         }
//         fclose(file);
//     }
//     cJSON *parsed = cJSON_Parse(data);
//     if (!parsed){
//         printf("\033[?1049l");
//         printf("Error parsing user-inf.json file!\n");
//         free(data);
//         return General;
//     }
//     cJSON *Readtime = cJSON_GetObjectItem(parsed, "Readtime");
    
//     General.Readtime = (int)cJSON_GetNumberValue(Readtime);
    
//     free(data);
//     cJSON_Delete(parsed);
//     return General;
// }


// int ReadUserinfReadtime(){
//     userinf General = OpenUserinf();
//     return General.Readtime;
// }


int exec(char* arg, PSH_GlobalFlags* flags,ShellMemory* mem,List* addons);
bool handleCommand(char *cmd, PSH_GlobalFlags *flags, ShellMemory *mem, List* addons);
void init_buffer(DynamicBuffer *db, size_t initial_capacity);
void append_to_buffer(DynamicBuffer *db, const char *new_data, size_t data_len);
void free_buffer(DynamicBuffer *db);
void editorRefreshScreen();
void editorProcessKeypress();
void editorClose();
/*--------------------------------------------*/



char* getDynamicInput() {
    DynamicBuffer db;
    init_buffer(&db, 32);

    int ch;
    while ((ch = getchar()) == '\n' || ch == '\r' || ch == EOF) {
        if (ch == EOF) {
            free_buffer(&db);
            return NULL;
        }
    }

    if (ch != EOF) {
        char c = (char)ch;
        append_to_buffer(&db, &c, 1);
    }
    while ((ch = getchar()) != '\n' && ch != '\r' && ch != EOF) {
        char c = (char)ch;
        append_to_buffer(&db, &c, 1);
    }

    return db.data;
}

void Stop(int sig){
    (void)sig;
    Exit = !Exit;
}

bool is_empty(const char *s) {
    while (*s) {
        if (!isspace((unsigned char)*s)) return false;
        s++;
    }
    return true;
    
}

void init_buffer(DynamicBuffer *db, size_t initial_capacity) {
    db->capacity = initial_capacity > 0 ? initial_capacity : 16;
    db->size = 0;
    db->data = (char *)malloc(db->capacity);
}



int MoveFileFunct (char* filepatch, char* newpath){
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








void append_to_buffer(DynamicBuffer *db, const char *new_data, size_t data_len) {
    while (db->size + data_len >= db->capacity) {
        db->capacity += 16; //Add 16 bytes 
        char *temp = (char *)realloc(db->data, db->capacity);
        if (!temp) {
            fprintf(stderr, "\033[38;2;209;0;0mError\033[0m\n");
            return;
        }
        db->data = temp;
    }
    
    memcpy(db->data + db->size, new_data, data_len);
    db->size += data_len;
    db->data[db->size] = '\0'; 
}

void free_buffer(DynamicBuffer *db) {
    free(db->data);
    db->data = NULL;
    db->size = db->capacity = 0;
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
int mov(char* CmdSource, ShellMemory* mem){
            char *arg = CmdSource + 4;
        while (*arg == ' ') arg++;
        if (*arg == '\0') {
            printf("Usage: mov <source> <destination>\n");
            free(CmdSource);
            return true;
        }

        char *arg2 = arg;
        while (*arg2 != ' ' && *arg2 != '\0') arg2++;
        if (*arg2 == '\0') {
            printf("Usage: mov <source> <destination>\n");
            free(CmdSource);
            return true;
        }

        *arg2 = '\0';
        char *new_name = arg2 + 1;
        while (*new_name == ' ') new_name++;
        if (*new_name == '\0') {
            printf("Usage: mov <source> <destination>\n");
            free(CmdSource);
            return true;
        }

        char *old_path = NULL;
        if (arg[0] == '/' || arg[0] == '\\' || (isalpha((unsigned char)arg[0]) && arg[1] == ':')) {
            old_path = strdup(arg);
        } else {
            old_path = CombinePath(mem->data, arg);
        }
        if (!old_path) { printf("Memory error\n"); free(CmdSource); return true; }

        /* compute src basename for case when destination is a directory */
        const char *src_basename = strrchr(arg, '/');
        const char *bslash = strrchr(arg, '\\');
        if (bslash && (!src_basename || bslash > src_basename)) src_basename = bslash;
        if (src_basename) src_basename++; else src_basename = arg;

        /* prepare destination path */
        char *dst_path = NULL;
        if (new_name[0] == '/' || new_name[0] == '\\' || (isalpha((unsigned char)new_name[0]) && new_name[1] == ':')) {
            /* absolute destination */
            if (is_directory(new_name)) {
                size_t s = strlen(new_name) + 1 + strlen(src_basename) + 1;
                dst_path = malloc(s);
                if (dst_path) snprintf(dst_path, s, "%s/%s", new_name, src_basename);
            } else {
                dst_path = strdup(new_name);
            }
        } else {
            /* relative destination -> combine with current dir first */
            char *candidate = CombinePath(mem->data, new_name);
            if (!candidate) { free(old_path); printf("Memory error\n"); free(CmdSource); return true; }
            if (is_directory(candidate)) {
                size_t s = strlen(candidate) + 1 + strlen(src_basename) + 1;
                dst_path = malloc(s);
                if (dst_path) snprintf(dst_path, s, "%s/%s", candidate, src_basename);
                free(candidate);
            } else {
                dst_path = candidate; /* already combined */
            }
        }

        if (!dst_path) { printf("Memory error\n"); free(old_path); free(CmdSource); return true; }

        if (MoveFileFunct(old_path, dst_path) == 0) {
            printf("\033[38;2;33;198;0mDone\033[0m\n");
        }

        free(old_path);
        free(dst_path);
        free(CmdSource);
        return 1;
}
List* push(List* head, char* new_string) {

    List* new_node = (List*)malloc(sizeof(List));
    if (new_node == NULL) return head; 

    new_node->data = (new_string == NULL) ? NULL : strdup(new_string);
    

    new_node->next = head;


    return new_node;
}

// Вызов функции с "ничем" вместо строки:

void freeList(List* head) {
    List* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        
        free(temp->data); 
        free(temp);       
    }
}

int exec(char* arg,PSH_GlobalFlags* flags,ShellMemory* mem,List* addons){
            
        FILE *fp = fopen(arg, "rb");
        if (!fp) {
            printf("Failed to open File!\n");
            return -1;
        }
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        rewind(fp);
        char *buffer = (char*)malloc(size + 1);
        while (fgets(buffer, size + 1, fp)) {
            
            char *token = strtok(buffer, "\n\r");
            while (token != NULL) {
                
                while (*token == ' ') token++;
                
                if (!handleCommand(token, flags,mem,addons)) { fclose(fp); return 0;}
                token = strtok(NULL, "\n\r");
            }
        
        }
        
        fclose(fp);
        free(buffer);
        return 0;
}
// void ReadTime(){
//     printf("\033[?1049h");
//     time_t start_t, end_t;
//     double total_t;
//     char* Localization;
//     char* Localization2;
//     Localization = getSection("rdtime:readTimeGet",langFile);
//     Localization2 = getSection("rdtime:Exitphrase",langFile);
//     long LocalizationSize = count_symbols(Localization,strlen(Localization));
//     if(Localization || Localization != NULL || Localization2 || Localization2 != NULL){
//         printf("%s\n",Localization2);
//         printf("%s\n",Localization);
//     }
//     else{
//         printf("\033[?1049l");
//         printf("Error reading file with localization!\n");
//         return;
//     }
//     if (Localization2){free(Localization2);}
//     if (Localization){free(Localization);}
    
//     char*data;
//     signal(SIGINT, Stop);
//     start_t = time(NULL);
//     while (Exit != true) {
//         SLEEP_MS(100);
//     }
//     end_t = time(NULL);
//     total_t = difftime(end_t, start_t);
    
//     /*---JSON Write---*/
//     FILE *file = fopen("user-inf.json", "rb");
//     if (!file){
//         printf("\033[?1049l");
//         printf("Error opening user-inf.json file!\n");
//         return;
//     }
//     else {
//         fseek(file, 0, SEEK_END);
//         long length = ftell(file);
//         fseek(file, 0, SEEK_SET);
//         data = (char*)malloc(length + 1);
//         if (data){
//             fread(data, 1, length, file);
//             data[length] = '\0';
//         }
//         fclose(file);


//     }
//     cJSON *parsed = cJSON_Parse(data);
//     if (!parsed){
//         printf("\033[?1049l");
//         printf("Error parsing user-inf.json file!\n");
//         free(data);
//         return;
//     }
//     cJSON *Readtime = cJSON_GetObjectItem(parsed, "Readtime");
    
//     /*--------Readtime formula--------*/
//     int seconds = (int)cJSON_GetNumberValue(Readtime);
//     int readtime_calc = (seconds > 0) ? (12000.0 / seconds) : 0;
//     if (readtime_calc){cJSON_SetNumberValue(Readtime, readtime_calc);}
    



    
//     char *out_str = cJSON_Print(parsed);
//     if (out_str) {
//         FILE *wfile = fopen("user-inf.json", "w"); 
//         if (wfile) {
//             fputs(out_str, wfile);
//             fclose(wfile);
//         }
//         free(out_str);
//     }
//     free(data);
//     cJSON_Delete(parsed);




//     fflush(stdout);
//     Exit = false;
//     printf("\033[?1049l");
// }
void parseGlobalFlags(char *cmd, PSH_GlobalFlags *flags) {
    
    if (flags->noFlag) {
        return;
    }

    char *pos;
    
    pos = strstr(cmd, "{q");
    if (pos) {
        flags->exitAfter = true;
        *pos = '\0';  
    }

    
    pos = strstr(cmd, "{d");
    if (pos) {
        flags->debugMode = true;
        *pos = '\0';
    }
    pos = strstr(cmd, "{s");
    if (pos) {
        flags->shutDownAfter= true;
        *pos = '\0';
        
    }

    cmd[strcspn(cmd, "\n")] = '\0';
    while (strlen(cmd) > 0 && cmd[strlen(cmd)-1] == ' ')
        cmd[strlen(cmd)-1] = '\0';
    }

bool handleCommand(char *cmd, PSH_GlobalFlags *flags,ShellMemory *mem,List* addons) {
    if (is_empty(cmd)) return true;
    char *CmdSource = strdup(cmd);
    if (!CmdSource) return true;
    strToLower(cmd);

    cmd[strcspn(cmd, "\n")] = '\0';
    while (strlen(cmd) > 0 && cmd[strlen(cmd)-1] == ' ')
        cmd[strlen(cmd)-1] = '\0';

    // if (strcmp(cmd, "windows-window-creator") == 0) {

        
    //     #ifndef _WIN32
    //         printf("This Module cant run on your OS! (Only Windows)\n");
    //         
    //     #else

    //         if (WindowsWindowCreator == false){
    //             WindowsWindowCreator = true;

    //         }
    //         else{WindowsWindowCreator = false;}
    //         printf("\033[38;2;33;198;0mDone\033[0m\n");
    //         
    //     #endif
    // }
    

    if (strcmp(cmd,"hello") == 0) {
        printf("Hello World!\n");
        
    }
    else if (strcmp(cmd, "cls") == 0) {
        #ifdef OS_LINUX
            system("clear");
        #else
            system("cls");
        #endif
    }
    else if (strcmp(cmd, "cpu-load") == 0) {
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
    }
    
    else if (strcmp(cmd, "ver") == 0) {

        printf("\n⠀ ⢀⣴⡿⠛⠿⣶⡀⠀⠀⠀⠀⠀⣰⡾⠟⠻⣷⡄⠀⠀⠀⠀⠀⣠⡾⠟⠛⢿⡆\n  ⣾⡏⠀⠀⠀⣿⡇⠀⠀⠀⠀⣸⡟⠀⠀⠀⣿⡇⠀⠀⠀⠀⠀⣿⡇⠀⢀⣾⠇\n⢰⣿⠀⠀⠀⢠⣿⠁⠀⠀⠀⢀⣿⠃⠀⠀⠀⣿⡇⠀⠀⠀⠀⠀⣨⣿⢿⣿⡁\n⢸⣿⠀⠀⠀⣼⡟⠀⠀⠀⠀⢸⣿⠀⠀⠀⣸⡿⠀⠀⠀⠀⢠⣾⠋⠀⠀⢹⣿⠀\n⠘⣿⣤⣤⣾⠟⠀⠀⣶⡆⠀⠈⢿⣦⣤⣴⠿⠁⠀⣴⡶⠀⠘⢿⣤⣤⣤⡾⠏\n⠀⠀⠉⠉⠀⠀⠀⠀⠉⠀⠀⠀⠀⠈⠉⠀⠀⠀⠀⠈⠀⠀⠀⠀⠈⠉⠁⠀⠀");
        printf("\033[38;2;160;0;209mProggShell\033[0m v0.0.8\n");
        fflush(stdout);
        
        
    }
    // Path commands
    else if (strncmp(cmd, "cd ", 3) == 0) {
        char *path_arg = CmdSource + 3; 
        while (*path_arg == ' ') path_arg++; 
        
        if (*path_arg != '\0') {
            psh_change_dir(path_arg, mem); 
        }
        free(CmdSource);
        
        return true;
    }
    // else if (strcmp(cmd, "run-ext") == 0) {

    //     exec_EXT(mem->data,1,0); 
        
    //     free(CmdSource);
        
    //     return true;
    // }
    else if (strncmp(cmd, "mdr ", 4) == 0) {
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
        free(CmdSource);
        return true;
    }
    else if (strncmp(cmd, "crt ", 4) == 0) {
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
                printf("\033[38;2;209;0;0mError: %s\033[0m\n",strerror(errno));
            }
        }
        free(FullPath);
        free(CmdSource);
        
        return true;
    }
    else if (strncmp(cmd, "cop ", 4) == 0 || strncmp(cmd, "cln ", 4) == 0) {
        /* Usage: cop <source> <destination>
           Supports: cop, cln (alias) */
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
            printf("\033[38;2;209;0;0mError: source and destination are the same\033[0m\n");
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
        init_buffer(&buf,8192);
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
            printf("\033[38;2;33;198;0mDone\033[0m\n");
        }

        /* free the temporary buffer */
        free_buffer(&buf);
        free(srcPath);
        free(dstPath);
        free(CmdSource);
        return true;
    }

    else if (strncmp(cmd, "mov ", 4) == 0) {
        mov(CmdSource, mem);
    }
    else if (strncmp(cmd, "fl", 4) == 0||strncmp(cmd, "dir", 4) == 0||strncmp(cmd, "ls", 4) == 0) {
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
    }
    
    // else if (strcmp(cmd, "readtm") == 0) {
    //     ReadTime();
    // }
    else if (strncmp(cmd, "ren ", 3) == 0) {
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

        free(CmdSource);
        
        return true;
    }
    
    
    else if (strcmp(cmd, "clv") == 0) {
        
        printf("\033[2J\033[H");
        fflush(stdout); 
    }
    else if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0 ) {
        free(CmdSource);
        return false;
    }
    else if (strncmp(cmd, "create", 6) == 0) {
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
                    SLEEP_MS(250);
                }
                
            }
            else if (strcmp(arg, "graph") == 0) {
                printf("\033[?1049h");
                for (size_t i = 0; i < 100; i++)
                {
                    CreateGraph(20,i, 1, 0, "|", "└","┌", "#", "·" ,0,0,95,0);
                    SLEEP_MS(10);
                    printf("\033[H");
                    printf("\033[K");
                    
                }
                printf("\033[?1049l");
            }
        }
        
    }
    else if (strncmp(cmd, "del ", 4) == 0) {
        char *arg = CmdSource + 4;  
        while (*arg == ' ') arg++;  

        if (*arg == '\0') {
            printf("\033[38;2;209;0;0mError: %s\033[0m\n",strerror(errno));
        } else {
            
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
                printf("\033[38;2;209;0;0mError: %s\033[0m\n",strerror(errno));
            } else {
                printf("\033[38;2;33;198;0mFile successfully deleted!\033[0m\n");
            }
            free_buffer(&FullPath);
        }
    }
    else if (strncmp(cmd, "prtread ", 8) == 0) {
        // int readtime_calc = ReadUserinfReadtime();
        char *arg = cmd + 8;      
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
    else if (addons != NULL && strcmp(CmdSource,addons->data)== 0 ){ // Addons System
        char* AddonPath = CombinePath(mem->data,addons->data);
        exec(AddonPath,flags,mem,NULL);
        

    }
    // No-Exist flag system
    else if (strcmp(cmd, "noflag") == 0) {
        flags->noFlag = true;
        printf("Flags disabled\n");
    }
    else if (strcmp(cmd, "exflag") == 0) {
        flags->noFlag = false;
        printf("Flags enabled\n");
    }

    else if (strncmp(cmd, "prt ", 4) == 0) {
        char *arg = CmdSource + 4;
        while (*arg == ' ') arg++;
        if (*arg == '\0'){
            printf("\n");
        }
        else {
            printf("%s\n" , arg);
        }
        
    }
    // else if (strncmp(cmd, "get ", 4) == 0) {
    //     char *arg = CmdSource + 4;
    //     while (*arg == ' ') arg++;
    //     if (*arg == '\0'){
            
    //         printf("[Download]: Error downloading '%s' use https or http\n", arg);
    //     }
    //     else {
    //         char filename[256];
    //         get_filename_from_url(arg, filename, sizeof(filename));
    //         printf("[Download]: Trying download %s ...\n" , filename);
    //         download(arg,filename);
    //         if (access(filename,'r'))
    //         {
                
    //             printf("[Download]: Download '%s' ended succefuly!\n", filename);
    //             
    //         }
    //         else{
    //             printf("[Download]: Download '%s' ended with errors!\n", filename);
    //             

    //         }
            
    //     }
        
    // }
    else if (strncmp(cmd, "run ", 4) == 0) {
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
            
                printf("\033[38;2;209;0;0mError executing command '%s' Error code: %d\033[0m\n\n" , arg , Status );
            }
        }
        
        
    }
    else if (strncmp(cmd, "exec", 4) == 0) {
        char *arg = CmdSource + 4;
        while (*arg == ' ') arg++;
        if (*arg == '\0'){
            printf("Usage <path_to_file>\n");
        }
        
        else {
            exec(arg,flags,mem,NULL);
        }
        
    }
    else if (strcmp(cmd, "cmd") == 0) {
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

    else if (strcmp(cmd, "mem-load") == 0) {
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

    else if (strncmp(cmd, "delay ", 6) == 0) {
        char *arg = CmdSource + 6;
        while (*arg == ' ') arg++;
        if (*arg == '\0'){
            printf("No time to wait!\n");
        }
        
        else {
            
            long ms = atol(arg);

            SLEEP_MS(ms);
        }
        
    }

    
    else if (strncmp(cmd, "set-lng", 7) == 0) {
        char *arg = CmdSource + 7;
        while (*arg == ' ') arg++;
        if (strlen(arg) == 0) {
            printf("\033[38;2;209;0;0mError! Specify language file\033[0m\n");
        } else {
            char* tempLang = CombinePath(ProgrammDir.data, arg); 
            
            if (access(tempLang,0)!= 0){
                printf("Error cant get read access to '%s'", arg);
                free(tempLang);
                
            }
            else{
                if (langFile) free(langFile);
                langFile = tempLang; 
                printf("Language file installed to: %s\n", langFile);
            }
        }
       
    }
    else if (strncmp(cmd, "neo ", 4) == 0) {
        char *arg = CmdSource + 4;
        while (*arg == ' ') arg++;

        if (strlen(arg) == 0) {
            printf("Usage: neo <file_path>\n");
        } else {
            
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
            printf("\n--- Neo closed ---\n");
        }
    }

    else if (strcmp(cmd, "help") == 0) {
        char* Temp = getSection("help", langFile);
        if (Temp == NULL || !Temp){
            printf("\033[38;2;209;0;0mError:The partition of language is free or does not exist\033[0m\n");
        }
        else{
            printf("%s",Temp);
            free(Temp);
        }
    }

    else if (strlen(cmd) > 0) {
        printf("Unknow command: %s\n", CmdSource);
    }
    
   
    if (!flags->noFlag) {
        if (flags->shutDownAfter) shutdownComputer();
        if (flags->debugMode) printf("(debug: executing '%s')\n", cmd);
        free(CmdSource);
        return !flags->exitAfter;
        
    }
    
    free(CmdSource);
    return true;
    
}



int main(int argc, char *argv[]) {
    
    ShellMemory current_path;
    init_start_path(&current_path);
    PSH_GlobalFlags flags = {0}; 
    init_buffer(&ProgrammDir,256);
    
    append_to_buffer(&ProgrammDir,current_path.data,strlen(current_path.data));
    #ifdef OS_WINDOWS
   
    setlocale(LC_ALL, ".UTF8");
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    
    
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    #endif
    
    langFile=CombinePath(ProgrammDir.data,"en-en.lng");
   
    
    if (argc > 1) {
        printf("Loading: %s\n", argv[1]);
        int Sccode = 0;
        
        FILE *file = fopen(argv[1], "r");
        if (file) {
            printf("Loaded file!\n");
            exec(argv[1],&flags,&current_path,NULL);
            fclose(file);
            Sccode = 0;
        } else {
            Sccode = 1;
            perror("\033[38;2;209;0;0mError: Cant open file!\033[0m");
        }   
    
        printf("Script Ended with code %d", Sccode);
        free(current_path.data);
        return(Sccode);
    }
    while (1) {
        printf("\n%s > ", current_path.data);
        char *input = getDynamicInput();

        if (!input) {
            printf("\033[38;2;209;0;0mMemory error!\033[0m\n");
            break;
        }
        
        
        
        flags = (PSH_GlobalFlags){ .noFlag = flags.noFlag };
        
        
        
        parseGlobalFlags(input, &flags);
        if (!handleCommand(input, &flags,&current_path,NULL)) {
            if (input) free(input);
            break;
        }
        free(input);
    }
    if (langFile) free(langFile);
    
    free_buffer(&ProgrammDir);
    free(current_path.data);
    return 0;
}