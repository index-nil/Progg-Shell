

/*-------------------------------------Progg Shell 2026 MIT License--------------------------------------*/


#include "modules/Commads.h" // Main Commands
#include <locale.h> //System language support
#include "modules/types.h"
#include "modules/DynamicTools.h"
#include "modules/fum.h" //File Utils Module import 
#include "modules/cJSON.h" //Json parser for user-inf.json
#include <stdio.h> // CMD main
#include <string.h> //String functions
#include <readline/readline.h>
#include <readline/history.h>//History save functions
#include <stdbool.h> //Bool variables
#include <stdlib.h>  // System control
#include <ctype.h>
#include "modules/print.h"
// #include "modules/ExtTools.h" //Extension Tools
// #define NCURSES_STATIC
#ifdef _WIN32
    #define F_OK 0
    #define access _access
    #define OS_WINDOWS
    #include <ncurses/ncurses.h> //If using MSYS2
    #include <windows.h> //Windows API
    #include <direct.h> //Folders functions
    #define GetCurrentDir _getcwd //Replace GetCurrentDir to _getcwd if we on Windows
    #define ChangeDir _chdir //Replace ChangeDir to _chdir if we on Windows
    #define WIN32_LEAN_AND_MEAN //less functions for faster compilation
    #define SLEEP_MS(x) Sleep(x) //Replace SLEEP_MS calls to Sleep

#else
    #include <ncurses.h> //If not using MSYS2
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
    #define GetCurrentDir getcwd //Replace GetCurrentDir to getcwd if we on linux
    #define ChangeDir chdir //Replace ChangeDir to chdir if we on Windows
    #define SLEEP_MS(x) usleep((x)*1000) //Replace SLEEP_MS calls to usleep
    #define OS_LINUX
#endif
#include <stddef.h>

/*--------------Variable create---------------*/

void init_buffer(DynamicBuffer *db, size_t initial_capacity);
void append_to_buffer(DynamicBuffer *db, const char *new_data, size_t data_len);
void free_buffer(DynamicBuffer *db);
bool handleCommand(char *cmd, PSH_GlobalFlags *flags, ShellMemory *mem);




char *psh_commands[] = {
    "hello", "cls", "cpu-load", "cd", "ver", "mdr", "crt", "cop", 
    "mov", "fl", "dir", "ls", "ren", "clv", "q", "quit", 
    "exit", "create", "del", "prtread", "prt", "run", "exec", 
    "term", "mem-load", "delay", "set-lng", "neo", "help", "if", 
    "noflag", "exflag", "export", "clh", NULL
};



/*------------Survivors Club (⌐■_■)-----------*/
DynamicBuffer ProgrammDir;
char* langFile;
/*--------------------------------------------*/



char *command_generator(const char *text, int state) {
    static int list_index, len;
    char *name;
    if (!state) {
        list_index = 0;
        len = strlen(text);
    }
    while ((name = psh_commands[list_index])) {
        list_index++;
        if (strncmp(name, text, len) == 0) return strdup(name);
    }
    return NULL;
}


char **psh_completion(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, command_generator);
}


bool is_empty(const char *s) { //String is empty check
    while (*s) {
        if (!isspace((unsigned char)*s)) return false;
        s++;
    }
    return true;
    
}





List* push(List* head, char* new_string) {

    List* new_node = (List*)malloc(sizeof(List));
    if (new_node == NULL) return head; 

    new_node->data = (new_string == NULL) ? NULL : strdup(new_string);
    

    new_node->next = head;


    return new_node;
}

void freeList(List* head) {
    List* temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        
        free(temp->data); 
        free(temp);       
    }
}

int exec(char* arg,PSH_GlobalFlags* flags,ShellMemory* mem){ //Script execute function
            
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
            //if (strncmp(token, "@SKIPSTR", 8) == 0) {
            //    continue;
            //}
            if (!handleCommand(token, flags,mem)) { 
                free(buffer);
                fclose(fp);
                return 0;
                
            }
            token = strtok(NULL, "\n\r");
        }
    
    }
    
    fclose(fp);
    free(buffer);
    return 0;
}

/*
StringVariable* add_string_var(StringVariable* head, char* name, char* value) {
    StringVariable* new_var = (StringVariable*)malloc(sizeof(StringVariable));
    if (new_var == NULL) return head;

    new_var->name = strdup(name);
    new_var->value = strdup(value);
    new_var->next = head; // Теперь типы совпадут

    return new_var;
}*/


void parseGlobalFlags(char *cmd, PSH_GlobalFlags *flags) { //Main flags parse function
    
    if (flags->noFlag) { // If no flag enabled exit from function
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

bool handleCommand(char *cmd, PSH_GlobalFlags *flags,ShellMemory *mem) { //Main command call function
    if (is_empty(cmd)) return true;
    char *CmdSource = strdup(cmd);
    if (!CmdSource) return true;
    strToLower(cmd); // Make command lowercase for HeLlO be a command also for better parsing

    cmd[strcspn(cmd, "\n")] = '\0';
    while (strlen(cmd) > 0 && cmd[strlen(cmd)-1] == ' ') {cmd[strlen(cmd)-1] = '\0';}
    


    if (strcmp(cmd,"hello") == 0) {
        printf("Hello World!\n"); // Just hello world
        
    }
    else if (strncmp(cmd, "cls",3) == 0) {
        cls(); // call clear screen function (OS addiction)
    }
    else if (strcmp(cmd, "cpu-load") == 0) {
        cpuload(); // call cpu load function
    }
    else if (strncmp(cmd, "cd ", 3) == 0) {
        cd(CmdSource,mem); // call enter to folder function with CmdSource and shell memory as arguments
    }
    else if (strcmp(cmd, "ver") == 0) { // Show current varsion of ProggShell with ascii art
        ver();
    }
    else if (strncmp(cmd, "mdr ", 4) == 0) {
        mdr(CmdSource, mem); //call make dir function with CmdSource and mem as arguments
    }
    else if (strncmp(cmd, "crt ", 4) == 0) {
        crt(CmdSource, mem); //call file create function with CmdSource and mem as arguments
    }
    else if (strncmp(cmd, "cop ", 4) == 0 || strncmp(cmd, "cln ", 4) == 0) {
        copy(CmdSource, mem); // call copy function with CmdSource and mem as arguments
    }
    else if (strncmp(cmd, "mov ", 4) == 0) {
        mov(CmdSource, mem); // call move function with CmdSource and mem as arguments
    }
    else if (strncmp(cmd, "fl", 4) == 0||strncmp(cmd, "dir", 4) == 0||strncmp(cmd, "ls", 4) == 0) {
        dir(mem); // call dir function with mem as argument
    }
    else if (strncmp(cmd, "ren ", 3) == 0) {
        ren(CmdSource, mem); // call raname function wirh CmdSource and shell memory as arguments
    }
    else if (strcmp(cmd, "clv") == 0) {
        printf("\033[2J\033[H"); // Fill screen with ' '
        fflush(stdout); 
    }
    else if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0 ) {
        free(CmdSource); // free CmdSource
        return false; // End Session
    }
    else if (strncmp(cmd, "create", 6) == 0) {
        create(cmd); // Create object (For testing)
    }
    else if (strncmp(cmd, "del ", 4) == 0) {
        del(CmdSource, mem); //Call delete function with source input and current path
    }
    else if (strncmp(cmd, "read ", 5) == 0) {
        prtread(CmdSource); // Read file and print 
        
    }
    else if (strncmp(cmd, "prtread ", 8) == 0) {
        prtread(CmdSource); // Read file and print with Words and Sumbols check
        
    }
    else if (strncmp(cmd, "prt ", 4) == 0) {
        prt(CmdSource); // print argument
    }
    else if (strncmp(cmd, "run ", 4) == 0) {
        run(CmdSource); //Ask system do a argument
    }
    else if (strncmp(cmd, "exec", 4) == 0) {
        execCommand(CmdSource,flags,mem); // Call Script execute function
    }
    else if (strcmp(cmd, "term") == 0) {
        CmdOpen(); //Call system terminal
    }
    
    else if (strcmp(cmd, "mem-load") == 0) {
        memload(); //Call memory load monitor
    }
    
    else if (strncmp(cmd, "delay ", 6) == 0) {
        delay(cmd); // Wait some time
    }
    else if (strncmp(cmd, "set-lng ",8) == 0) {
        if (setlng(CmdSource)== 0){langFile=ReadUserinfLanguage();} // set default language and write it to user-inf.lng and Recalculate language file
    }
    else if (strncmp(cmd, "neo ", 4) == 0) {
        NeoOpen(CmdSource, mem); // Call neo
    }
    else if (strncmp(cmd, "help",4) == 0) {
        help(); // Show all commands from language file
    }
    else if (strncmp(cmd, "if ", 3) == 0) {
        ifFunct(CmdSource); // execute logical if
    }
    /*else if (strncmp(cmd, "@stringvarcreate ", 17) == 0) { //Creating User String Variable
        stringVarCreateCommand(CmdSource,vars);
    }*/
    // No-Exist flag system
    else if (strcmp(cmd, "noflag") == 0) { //Disable flags
        flags->noFlag = true;
        printf("Flags disabled\n");
    }
    else if (strcmp(cmd, "exflag") == 0) {// Enable flags
        flags->noFlag = false;
        printf("Flags enabled\n");
    }
    else if (strncmp(cmd,"export ",7)== 0){ //Export cmd history to file
        export_file(CmdSource,mem);
    }
    else if (strcmp(cmd,"clh") == 0){ //Clear cmd history
        clear_history();
    }
    //else if (strcmp(cmd,"recalc") == 0){ //Recalculate mutable variables
    //    langFile=ReadUserinfLanguage();
    //}
    //else if (strcmp(cmd, "readtm") == 0) {
    //    ReadTime(); /*Testing!*/
    //}

    // else if (strcmp(cmd, "run-ext") == 0) {
    //     exec_EXT(mem->data,1,0); 
    //     free(CmdSource);
    //     return true;
    // }
    // else if (addons != NULL && strcmp(CmdSource, addons->data) == 0) { // Addons System 
    //     List* cur = addons;
    //     bool found = false;
    //     while (cur != NULL) {
    //         if (cur->data && strcmp(CmdSource, cur->data) == 0) {
    //             char* AddonPath = CombinePath(mem->data, cur->data);
    //             exec(AddonPath, flags, mem, NULL, NULL);
    //             free(AddonPath);
    //             found = true;
    //             break;
    //         }
    //         cur = cur->next;
    //     }

        
    // }
    

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
    
    else if (strlen(cmd) > 0) { //if cant find command show error
        printf("Unknow command: %s\n", CmdSource);
    }
    
    
    if (!flags->noFlag) { // run flags if noFlag not activated
        if (flags->shutDownAfter) shutdownComputer();
        if (flags->debugMode) printf("[debug]: executing '%s'\n", cmd);
        free(CmdSource);
        return !flags->exitAfter;
        
    }
    
    free(CmdSource); //Clean up after running
    return true; // Continue session
    
}



int main(int argc, char *argv[]) {
    
    ShellMemory current_path; // Path System
    init_start_path(&current_path); // Creating dynamic buffer with current path
    PSH_GlobalFlags flags = {0};  // Initilization flags
    init_buffer(&ProgrammDir,256); // Creating Dynamic Buffer with programm folder
    
    append_to_buffer(&ProgrammDir,current_path.data,strlen(current_path.data)); // Set Path to Proggramm folder
    #ifdef OS_WINDOWS // Windows UTF-8 support
   
    setlocale(LC_ALL, ".UTF8");
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);
    
    
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    #endif

    langFile=ReadUserinfLanguage(); //Get language path from user-inf.json
    //StringVariable* vars = NULL;
    
    if (argc > 1) { // If Progg Shell started with argument (Script path) Progg shell trying to execute it
        printf("Loading: %s\n", argv[1]);
        int Sccode = 0;
        
        FILE *file = fopen(argv[1], "r");
        if (file) {
            printf("Loaded file!\n");
            exec(argv[1],&flags,&current_path);
            fclose(file);
            Sccode = 0;
        } else {
            Sccode = 1;
            error("Cant open file!");
        }   
        
        printf("Script Ended with code %d\n", Sccode);
        free(current_path.data);
        return(Sccode);
    }
    #ifdef _WIN32
        if (!SetConsoleTitle("Progg Shell")){printf("[Warning]: Cant rename console title!\n");}
    #endif
    rl_attempted_completion_function = psh_completion;
    while (1) {
        
        DynamicBuffer db_prompt;
        init_buffer(&db_prompt, 64); 
        append_to_buffer(&db_prompt, "\n", 1);
        append_to_buffer(&db_prompt, current_path.data, strlen(current_path.data)); 
        append_to_buffer(&db_prompt, " > ", 3);

   
        char *input = readline(db_prompt.data);

        free_buffer(&db_prompt); 
        if (!input) {
            error("Memory allocation error!");
            break;
        }

       
        if (strlen(input) > 0) {
            
            add_history(input);

            flags = (PSH_GlobalFlags){ .noFlag = flags.noFlag };
            parseGlobalFlags(input, &flags);

            if (!handleCommand(input, &flags, &current_path)) {
                free(input);
                break;
            }
        }
        free(input);
    }

    if (langFile) free(langFile); // Clear after session
    clear_history();
    free_buffer(&ProgrammDir);
    free(current_path.data);
    return 0;
}
