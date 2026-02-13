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
// #include "modules/cJSON.h" //JSON read
#include "modules/Sys_load.h"
// #include "modules/mongoose.h"
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
    #include <time.h>
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
    char *data;     // Сами данные
    size_t size;    // Сколько байт сейчас реально занято
    size_t capacity; // Общий размер выделенного блока
} DynamicBuffer;

typedef struct {
    bool noFlag; //{n
    bool exitAfter;   // {q
    bool shutDownAfter; //{s
    bool debugMode;   // {d
} PSH_GlobalFlags;

typedef struct {
    char *data;      // Указатель на строку в куче
    size_t capacity; // Текущий размер выделенной памяти
} ShellMemory;

CpuMonitor monitor;
float cores_load[MAX_CORES];
char* langFile;
DynamicBuffer ProgrammDir;
volatile sig_atomic_t Exit = false;

// #ifdef _WIN32
//     bool WindowsWindowCreator = false;
// #endif

int exec(char* arg, PSH_GlobalFlags* flags,ShellMemory* mem);
bool handleCommand(char *cmd, PSH_GlobalFlags *flags, ShellMemory *mem);
void init_buffer(DynamicBuffer *db, size_t initial_capacity);
void append_to_buffer(DynamicBuffer *db, const char *new_data, size_t data_len);
void free_buffer(DynamicBuffer *db);
/*--------------------------------------------*/



char* getDynamicInput() {
    DynamicBuffer db;
    init_buffer(&db, 32); // Начинаем с малого

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        char c = (char)ch;
        append_to_buffer(&db, &c, 1);
    }

    if (db.size == 0 && ch == EOF) {
        free_buffer(&db);
        return NULL;
    }

    return db.data; // Возвращаем указатель. Владение переходит к вызывающему.
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
    // Копируем данные в конец
    memcpy(db->data + db->size, new_data, data_len);
    db->size += data_len;
    db->data[db->size] = '\0'; // Всегда держим нуль-терминатор в конце
}
// 3. Очистка
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
    // 1. Пытаемся сменить папку
    if (ChangeDir(target) == 0) {
        
        DynamicBuffer temp;
        // ОШИБКА БЫЛА ЗДЕСЬ: нужно передавать адрес (&), а не разыменование (*)
        init_buffer(&temp, 256); // Начнем с 256 байт, это разумный минимум

        // 2. Получаем текущую директорию "резиновым" способом
        // Мы пытаемся получить путь. Если GetCurrentDir возвращает NULL и ошибка ERANGE,
        // значит буфер мал. Увеличиваем и пробуем снова.
        while (GetCurrentDir(temp.data, temp.capacity) == NULL) {
            if (errno == ERANGE) {
                // Буфер мал — удваиваем размер
                temp.capacity *= 2;
                char *new_ptr = (char*)realloc(temp.data, temp.capacity);
                if (!new_ptr) {
                    perror("\033[38;2;209;0;0mMemory allocation error\033[0m\n");
                    free_buffer(&temp);
                    return;
                }
                temp.data = new_ptr;
            } else {
                // Другая ошибка (например, прав нет)
                perror("getcwd error\n");
                free_buffer(&temp);
                return;
            }
        }

        // 3. Обновляем основной ShellMemory (mem)
        // ОШИБКА БЫЛА ЗДЕСЬ: strlen требует char*, а не структуру. Используем temp.data
        size_t new_len = strlen(temp.data);
        
        if (new_len >= mem->capacity) {
            mem->capacity = new_len + 16;
            char *new_mem_data = (char*)realloc(mem->data, mem->capacity);
            // Всегда проверяй realloc!
            if (new_mem_data) {
                mem->data = new_mem_data;
            }
        }
        
        // Копируем данные из буфера temp в память шелла
        strcpy(mem->data, temp.data);
        
        // 4. Очищаем временный буфер
        free_buffer(&temp);

    } else {
        // Ошибка смены директории (нет такой папки и т.д.)
        // Лучше выводить, какую папку не нашли
        fprintf(stderr, "Error changing dir to '%s': %s\n", target, strerror(errno));
    }
}

int exec(char* arg,PSH_GlobalFlags* flags,ShellMemory* mem){
            
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
                // Очистка от ведущих пробелов
                while (*token == ' ') token++;
                
                if (!handleCommand(token, flags,mem)) { fclose(fp); return 0;}
                token = strtok(NULL, "\n\r");
            }
        
        }
        
        fclose(fp);
        free(buffer);
        return 0;
}

void parseGlobalFlags(char *cmd, PSH_GlobalFlags *flags) {
    
    if (flags->noFlag) {
        return;
    }

    char *pos;
    
    pos = strstr(cmd, "{q");
    if (pos) {
        flags->exitAfter = true;
        *pos = '\0';  // убираем из команды
    }

    // проверяем {d
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

    // убираем пробелы в конце команды
    cmd[strcspn(cmd, "\n")] = '\0';
    while (strlen(cmd) > 0 && cmd[strlen(cmd)-1] == ' ')
        cmd[strlen(cmd)-1] = '\0';
    }

bool handleCommand(char *cmd, PSH_GlobalFlags *flags,ShellMemory *mem) {
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
                    sprintf(core_label, "[Core %2d]", i); // %2d выровняет Core 9 и Core 10

                    // Чтобы 0% не вызывало ошибку в вашей функции, передаем минимум 1 или правим функцию
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

        printf("\n⠀⢀⣴⡿⠛⠿⣶⡀⠀⠀⠀⠀⠀⣰⡾⠟⠻⣷⡄⠀⠀⠀⠀⠀⣠⡾⠟⠛⢿⡆\n ⠀⣾⡏⠀⠀⠀⣿⡇⠀⠀⠀⠀⣸⡟⠀⠀⠀⣿⡇⠀⠀⠀⠀⠀⣿⡇⠀⢀⣾⠇\n⢰⣿⠀⠀⠀⢠⣿⠁⠀⠀⠀⢀⣿⠃⠀⠀⠀⣿⡇⠀⠀⠀⠀⠀⣨⣿⢿⣿⡁\n⢸⣿⠀⠀⠀⣼⡟⠀⠀⠀⠀⢸⣿⠀⠀⠀⣸⡿⠀⠀⠀⠀⢠⣾⠋⠀⠀⢹⣿⠀\n⠘⣿⣤⣤⣾⠟⠀⠀⣶⡆⠀⠈⢿⣦⣤⣴⠿⠁⠀⣴⡶⠀⠘⢿⣤⣤⣤⡾⠏\n⠀⠀⠉⠉⠀⠀⠀⠀⠉⠀⠀⠀⠀⠈⠉⠀⠀⠀⠀⠈⠀⠀⠀⠀⠈⠉⠁⠀⠀");
        printf("\033[38;2;160;0;209mProggShell\033[0m v0.0.8\n");
        
        
    }
    // Path commands
    else if (strncmp(cmd, "cd ", 3) == 0) {
        char *path_arg = CmdSource + 3; // Берем путь из оригинала (чтобы регистр не портился)
        while (*path_arg == ' ') path_arg++; // Пропускаем пробелы
        
        if (*path_arg != '\0') {
            psh_change_dir(path_arg, mem); // Вызываем твою новую функцию
        }
        free(CmdSource);
        
        return true;
    }
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
    else if (strncmp(cmd, "fl", 4) == 0||strncmp(cmd, "dir", 4) == 0||strncmp(cmd, "ls", 4) == 0) {
        char *folder_name = strrchr(mem->data, '/'); 

        // Если на Windows используются обратные слэши, проверим и их
        if (!folder_name) folder_name = strrchr(mem->data, '\\');

        if (folder_name) {
            // strrchr возвращает указатель на сам слэш, поэтому прибавляем 1
            folder_name++; 
        } else {
            // Если слэшей нет, значит мы в корне или имя простое
            folder_name = mem->data;
        }
        struct dirent *entry;
        DIR *dp;

        // Открываем текущую директорию из памяти шелла
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
        char *arg = CmdSource + 4;      // берем оригинальный ввод для аргумента
        while (*arg == ' ') arg++;  // пропускаем пробелы

        if (*arg == '\0') {
            printf("\033[38;2;209;0;0mError: %s\033[0m\n",strerror(errno));
        } else {
            
            DynamicBuffer FullPath; //Made for future
            init_buffer(&FullPath,256);
            append_to_buffer(&FullPath, mem->data, strlen(mem->data));
            
            // 2. Добавляем разделитель (слэш)
            // 2. Добавляем разделитель, только если его еще нет в конце mem->data
            size_t path_len = strlen(mem->data);
            if (path_len > 0 && mem->data[path_len - 1] != '/' && mem->data[path_len - 1] != '\\') {
                #ifdef OS_WINDOWS
                    append_to_buffer(&FullPath, "\\", 1);
                #else
                    append_to_buffer(&FullPath, "/", 1);
                #endif
            }
            // 3. Добавляем имя файла
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
        char *arg = cmd + 8;      // берем оригинальный ввод для аргумента
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
                    printf(Localization, words,symbols);
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
            exec(arg,flags,mem);
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
            
            // 2. Считаем SWAP (Windows Logic: PageFile - Physical RAM)
            // Добавляем проверку, чтобы не было гигантских чисел
            unsigned long long total_swap_mb = 0;
            unsigned long long used_swap_mb = 0;

            if (memload.swap > 0) {
                total_swap_mb = memload.swap / 1024 / 1024;
                // Защита от отрицательного результата
                if (memload.swap >= memload.freeswap) {
                    used_swap_mb = (memload.swap - memload.freeswap) / 1024 / 1024;
                }
            }

            
            printf("\033[H");
            
            printf("(^C To Exit)\n\n");
            printf("[Memory]:%llu MB\n\n",total_ram_mb);
            printf("[SWAP]:%llu MB\n\n",total_swap_mb);
            if (used_ram_mb < (total_ram_mb / 100 * 50)){
                create_pb("[Memory Usage]:",1, (int)((used_ram_mb * 100) / total_ram_mb),20,"|","-","|",1,17,0,171);
            }
            else if (used_ram_mb > (total_ram_mb / 100 * 50)){
                create_pb("[Memory Usage]:",1, (int)((used_ram_mb * 100) / total_ram_mb),20,"|","-","|",1,255,208,0);
            }
            else if (used_ram_mb > (total_ram_mb / 100 * 80)){
                create_pb("[Memory Usage]:",1, (int)((used_ram_mb * 100) / total_ram_mb),20,"|","-","|",1,209,0,0);
            }
            printf("   %llu MB\n\n",used_ram_mb);
            
            
            if (used_swap_mb < (total_swap_mb / 100 * 50)){
                create_pb("[SWAP Usage]:  ",1, (int)((used_swap_mb * 100) / total_swap_mb),20,"|","-","|",1,17,0,171);
            }
            else if (used_swap_mb > (total_swap_mb / 100 * 50)){
                create_pb("[SWAP Usage]:  ",1, (int)((used_swap_mb * 100) / total_swap_mb),20,"|","-","|",1,255,208,0);
            }
            else if (used_swap_mb > (total_swap_mb / 100 * 80)){
                create_pb("[SWAP Usage]:  ",1, (int)((used_swap_mb * 100) / total_swap_mb),20,"|","-","|",1,209,0,0);
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
    PSH_GlobalFlags flags = {0}; // сохраняем между командами
    init_buffer(&ProgrammDir,256);
    
    append_to_buffer(&ProgrammDir,current_path.data,strlen(current_path.data));
    #ifdef OS_WINDOWS
    // Устанавливаем кодировку ввода и вывода в UTF-8 (65001)
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    
    // Включаем поддержку ANSI-последовательностей (для цвета текста)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    #endif
    
    setlocale(LC_ALL, "");
    langFile=CombinePath(ProgrammDir.data,"en-en.lng");

    if (argc > 1) {
        printf("Loading: %s\n", argv[1]);
        int Sccode = 0;
        // Здесь вы можете открыть файл для чтения
        FILE *file = fopen(argv[1], "r");
        if (file) {
            printf("Loaded file!\n");
            exec(argv[1],&flags,&current_path);
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
        if (!handleCommand(input, &flags,&current_path)) {
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