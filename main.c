#include <stdio.h> // CMD main
#include <string.h> //String functions
#include <stdbool.h> //Bool variables
#include <ctype.h> //Lower command library
#include <stdlib.h>  // System control
#include <errno.h> //Error number print 
#include <locale.h> //CMD set system language
#include "modules/fum.h" //File Utils Module import 
#include "modules/UIlib.h" //Progress bar, radio button and other
#include "modules/LngModule.h" //Localization
// #include "modules/cJSON.h" //JSON read
#include "modules/Sys_load.h"
// #include "modules/mongoose.h"
#include <signal.h>




#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define SLEEP_MS(x) Sleep(x)
    
#else
    #include <unistd.h>
    #define SLEEP_MS(x) usleep((x)*1000)
#endif

#ifdef _WIN32 //made for source code compilation on other systems
#define OS_WINDOWS

#elif __linux__
#define OS_LINUX
#include <unistd.h>
#include <time.h>
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
    bool noFlag; //{n
    bool exitAfter;   // {q
    bool shutDownAfter; //{s
    bool debugMode;   // {d
} PSH_GlobalFlags;

CpuMonitor monitor;
float cores_load[MAX_CORES];
char langFile[10] = "en-en.lng";
volatile sig_atomic_t Exit = false;

// #ifdef _WIN32
//     bool WindowsWindowCreator = false;
// #endif
int exec(char* arg, PSH_GlobalFlags* flags);
bool handleCommand(char *cmd, PSH_GlobalFlags *flags);
/*--------------------------------------------*/

char* getDynamicInput() {
    size_t size = 16; // Начальный размер буфера
    size_t len = 0;
    char *buffer = malloc(size);
    if (!buffer) return NULL;

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        buffer[len++] = ch;
        
        // Если место закончилось, увеличиваем буфер в 2 раза
        if (len + 1 >= size) {
            size += 16;
            char *new_buffer = realloc(buffer, size);
            if (!new_buffer) {
                free(buffer);
                return NULL;
            }
            buffer = new_buffer;
        }
    }
    buffer[len] = '\0'; // Завершающий ноль
    return buffer;
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

int exec(char* arg,PSH_GlobalFlags* flags){
            
        FILE *fp = fopen(arg, "r");
        if (!fp) {
            printf("Failed to open File!\n");
            return -1;
        }
        char buffer[2048];
        while (fgets(buffer, sizeof(buffer), fp)) {
            
            char *token = strtok(buffer, "\n\r");
            while (token != NULL) {
                // Очистка от ведущих пробелов
                while (*token == ' ') token++;
                
                if (!handleCommand(token, flags)) { fclose(fp); return 0;}
                token = strtok(NULL, "\n\r");
            }
        
        }
        
        fclose(fp);
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

bool handleCommand(char *cmd, PSH_GlobalFlags *flags) {
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
    //         printf("Done\n");
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

                    // overwrite ставим 2: это сделает \r в начале и \n в конце
                    // Color: 32 - зеленый, 33 - желтый, 31 - красный
                    int color = 32; 
                    if (val > 50) color = 33;
                    if (val > 80) color = 31;

                    create_pb(
                        core_label, // Текст "[Core 0]"
                        1,          // ShowPercent (1 = показать % в конце)
                        val,        // Значение нагрузки
                        20,         // Длина полоски (chars)
                        "#",        // Full char
                        "-",        // Void
                        "|",        // Bordes
                        0,          // overwrite=0
                        color       // Цвет (ANSI код)
                    );
                    
            }
        }
        fflush(stdout);
        Exit = false;
        printf("\033[?1049l");
    }
    
    else if (strcmp(cmd, "ver") == 0) {
        printf("ProggShell v0.0.7NU\n");
        
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
                        create_pb("Test", 1,i,10,"|","·","|",2,95);
                        break;
                    }
                    create_pb("Test", 1,i,10,"|","·","|",1,95);
                    #ifdef _WIN32
                        Sleep(100);
                    #else
                        usleep(100000);
                    #endif
                }
                
            }
            else if (strcmp(arg, "graph") == 0) {
                CreateGraph(20, 50, 1, 0, "|", "└","┌", "#", "·" ,0,0,95,0);
            }
        }
        
    }
    else if (strncmp(cmd, "del ", 4) == 0) {
        char *arg = CmdSource + 4;      // берем оригинальный ввод для аргумента
        while (*arg == ' ') arg++;  // пропускаем пробелы

        if (*arg == '\0') {
            printf("Error: Specify filename!\n");
        } else {
            char filename[256];
            strncpy(filename, arg, sizeof(filename)-1);
            filename[sizeof(filename)-1] = '\0';

            if (remove(filename) != 0) {
                printf("Error %d: %s\n", errno, strerror(errno));
            } else {
                printf("File successfully deleted!\n");
            }
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
                printf("Done\n");
            }
            else {
                printf("Error executing command '%s' Error code: %d\n" , arg , Status );
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
            exec(arg,flags);
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
            printf("Error! Error code: %d\n", Status );
        }
        else {
            printf("Returned to ProggShell\n");
        } 

        
    }

    else if (strncmp(cmd, "delay ", 6) == 0) {
        char *arg = CmdSource + 6;
        while (*arg == ' ') arg++;
        if (*arg == '\0'){
            printf("No time to wait!\n");
        }
        
        else {
            
            long ms = atol(arg);

            #ifdef _WIN32
                Sleep(ms);
            #else
                usleep(ms * 1000);
            #endif

        }
        
    }

    
    else if (strncmp(cmd, "set-lng", 7) == 0) {
        char filename[10];
        char *arg = cmd + 7;
        while (*arg == ' ') arg++;
        if (strlen(arg) == 0) {
            printf("Error! Specify language file!\n");
        } else if (strlen(arg) >= 10) {
            printf("Error: filename too long! Max %d chars\n", 10-1);
        } else {
            strncpy(filename, arg, sizeof(filename)-1);
            filename[sizeof(filename)-1] = '\0';

            strncpy(langFile, filename, sizeof(langFile)-1);
            langFile[sizeof(langFile)-1] = '\0';

            printf("Language file installed to: %s\n", langFile);
        }
    }

    else if (strcmp(cmd, "help") == 0) {
        char* Temp = getSection("help", langFile);
        if (Temp == NULL || !Temp){
            printf("The partition is free or does not exist | ERROR 100\n");
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
    PSH_GlobalFlags flags = {0}; // сохраняем между командами
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
    
    setlocale(LC_ALL, "ru_RU.UTF-8");
    
    if (argc > 1) {
        printf("Loading: %s\n", argv[1]);
        int Sccode = 0;
        // Здесь вы можете открыть файл для чтения
        FILE *file = fopen(argv[1], "r");
        if (file) {
            printf("Loaded file!\n");
            exec(argv[1],&flags);
            fclose(file);
            Sccode = 0;
        } else {
            Sccode = 1;
            perror("Error: Cant open file!");
        }   
    
        printf("Script Ended with code %d", Sccode);
        return(Sccode);
    }
    while (1) {
        printf("> ");
        char *input = getDynamicInput();

        if (!input) {
            printf("Memory error!\n");
            break;
        }
        
        
        
        flags = (PSH_GlobalFlags){ .noFlag = flags.noFlag };
        
        
        
        parseGlobalFlags(input, &flags);
        if (!handleCommand(input, &flags)) {
            if (input) free(input);
            break;
        }
        free(input);
    }
    
    return 0;
}