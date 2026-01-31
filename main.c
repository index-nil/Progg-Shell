#include <stdio.h> // CMD main
#include <string.h> //String functions
#include <stdbool.h> //Bool variables
#include <ctype.h> //Lower command library
#include <stdlib.h>  // System control
#include <errno.h> //Error number print 
#include <locale.h> //CMD set system language
#include "modules/fum.h" //File Utils Module import 
#include "modules/EXTbasiclib.h"


char langFile[10] = "en-en.lng";
#ifdef _WIN32 //made for source code compilation on other systems
    #define OS_WINDOWS
    #include <windows.h>
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


typedef struct {
    bool noFlag; //{n
    bool exitAfter;   // {q
    bool shutDownAfter; //{s
    bool debugMode;   // {d
} PSH_GlobalFlags;


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
    char CmdSource[512];
    strcpy(CmdSource , cmd );
    strToLower(cmd);

    cmd[strcspn(cmd, "\n")] = '\0';
    while (strlen(cmd) > 0 && cmd[strlen(cmd)-1] == ' ')
        cmd[strlen(cmd)-1] = '\0';

    if (strcmp(cmd, "hello") == 0) {
        
        printf("Hello World!\n");
    }
    else if (strcmp(cmd, "cls") == 0) {
        #ifdef OS_LINUX
            system("clear");
        #else
            system("cls");
        #endif
    }
    else if (strcmp(cmd, "ver") == 0) {
        printf("ProggShell v0.0.7NU\n");
    }
    else if (strcmp(cmd, "clv") == 0) {
        
        printf("\033[2J\033[H");
        fflush(stdout); 
    }
    else if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0 ) {
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
                    usleep(100000);
                }
                
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

        if (*arg == '\0') {
            printf("Error: Specify filename!\n");
        } else {
            long filesize = 0;
            char *content = read_file_to_buffer(arg, &filesize);

            if (content){
                printf("%s\n\n\n", content);
                int words = count_words(content, filesize);
                int symbols = count_symbols(content,filesize, false);
                printf("|PSh| Detected words in text: %d | Symbols in text %d | Read time for you: ~Null |", words,symbols);
                free(content);
            }
            else{
                printf("Can't open file!\n");
            }
          
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
        FILE *file = fopen(langFile, "r");
        if (!file) {
            perror("Failed to open language file !");
            printf("\nEnter 'set-lng (language file name).lng' to change language. (Language file need's to be in program folder!)");
            return !flags->exitAfter;
        }

        char line[256];
        while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, "$help", 5) == 0) {       //Lng help section finder
                continue;
            }
            else if (line[0] == '$'){
                break;
            }
            line[strcspn(line, "\n")] = 0;
            
            if (line[0] == '#') continue;
            printf("%s\n", line);
        }
        fclose(file);
    }
    else if (strlen(cmd) > 0) {
        printf("Unknow command: %s\n", CmdSource);
    }
    
   
    if (!flags->noFlag) {
        if (flags->shutDownAfter) shutdownComputer();
        if (flags->debugMode) printf("(debug: executing '%s')\n", cmd);
        return !flags->exitAfter;
    }

    
    return true;

}


int main() {
    char input[256];
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
    
    while (1) {
        printf("> ");
        fgets(input, sizeof(input), stdin);
        
        
        
        flags = (PSH_GlobalFlags){ .noFlag = flags.noFlag };

        
        
        parseGlobalFlags(input, &flags);

        if (!handleCommand(input, &flags)) {
            break;
        }
    }

    return 0;
}