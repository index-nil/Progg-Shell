

/*-------------------------------------Progg Shell MIT License--------------------------------------*/


/* Change log (Not Full)

--   Deleted unusefull Dynamic buffer
--   Added import commands from PATH
--   Added command separation tools
--   Removed ~90% commands from linux version
--   Deleted unusefull functions
--   Deleted flags system
--   Added Variable system
--   Deleted MacOS and other systems support except Windows, Linux

*/



#include "modules/Commads.h" // Main Commands
#include "modules/types.h"
#include "modules/DynamicTools.h"
#include "modules/fum.h" //File Utils Module import 
#include "modules/UIlib.h"
#include <string.h> //String functions
#include <stdio.h>
#include <stdlib.h>  // System control
#include <locale.h> //System language support
// #include <ctype.h>
#include "modules/UIlib.h"
#include "modules/print.h" //error, info, warn, ... functions
#include "modules/PATHtools.h"


#ifdef _WIN32
    #define F_OK 0
    #define WIN32_LEAN_AND_MEAN //less functions for faster compilation
    #define access _access
    #include <windows.h> //Windows API
    #include "inc/editline/readline.h"
    #include "inc/editline/wineditline.h"
    #include "inc/histedit.h"
    #include <io.h>
    // #include <direct.h> //Folders functions
    #define GetCurrentDir _getcwd //Replace GetCurrentDir to _getcwd if we on Windows
    #define ChangeDir _chdir //Replace ChangeDir to _chdir if we on Windows
    #define SLEEP_MS(x) Sleep(x) //Replace SLEEP_MS calls to Sleep

#else
    // #include <sys/types.h>
    #include <editline/readline.h> //readline too big boyyyyy and GPL license >:(
    #include <histedit.h>
    #include <unistd.h>
    #define GetCurrentDir getcwd //Replace GetCurrentDir to getcwd if we on linux
    #define ChangeDir chdir //Replace ChangeDir to chdir if we on Windows
    #define SLEEP_MS(x) usleep((x)*1000) //Replace SLEEP_MS calls to usleep
    #define OS_LINUX
#endif


/*--------------Variable initilization---------------*/

// Make prototypes
void init_buffer(DynamicBuffer *db, size_t initial_capacity);
void append_to_buffer(DynamicBuffer *db, const char *new_data, size_t data_len);
void free_buffer(DynamicBuffer *db);
unsigned char handleCommand(char *cmd, ShellMemory *mem);
// ---------------

char** psh_hist_entries = NULL;
int    psh_hist_count   = 0;
int    psh_hist_head    = 0;

Future futurecommand; 
int futureexecute = 0; // "future" command important variable made for check existence "future" commands





/*-------------------Variables Create-------------------*/

int MaxCommandsInHistory; //Max commands in terminal history from PSH_settings
double Readtime; //Read time value from PSH_settings (aka user-inf.json)
char* Settings;

Command* Commands_List;
char* Commands_Path_Massive;
unsigned int Commands_Count;


Variable * Variables_List;
unsigned int Variables_Count;
bool ignore_Variables = false;
bool Disable_PSH_Commands = false;

/*------------Survivors Club (⌐■_■)-----------*/

char* ProgrammDir; //Creating variable with Progg Shell dir Path     (v0.0.9 Dont make Dynamic buf for static path)
char* langFile; // Creating variable with language file path

/*--------------------------------------------*/

char *command_generator(const char *text, int state) {
    static unsigned long list_index;
    static size_t len;
    char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while (list_index < Commands_Count) {
        name = Commands_List[list_index].Name;
        list_index++;

        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}



char **psh_completion(const char *text, int start, int end) {
    (void)start; (void)end;
    return rl_completion_matches(text, command_generator);
}


int is_empty(const char *s) { //String is empty check
    while (*s) {
        if (!isspace((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
    
}






unsigned char handleCommand(char *cmd, ShellMemory *mem) { //Main command call function
    if (is_empty(cmd)) return 1;
    char *CmdSource = strdup(cmd);
    if (!CmdSource) return 1;
    
    /*---------------------Variable detection---------------------*/

    if (Variables_Count > 0 && ignore_Variables != true){ //Need to add more variables support
        
        char* var_start;

        for (size_t i = 0; i < Variables_Count; i++){
            
            char* buf = CombineStrings("#", Variables_List[i].name);
            
            strToLower(buf);

            CmdSource = str_replace_all(CmdSource, buf, Variables_List[i].data);
            
            
            free(buf);

        }
        




    }
    strToLower(cmd); // Make command lowercase for HeLlO be a command also for better parsing
    
    bool Command_is_Founded = false;

    
    cmd[strcspn(cmd, "\n")] = '\0';
    while (strlen(cmd) > 0 && cmd[strlen(cmd)-1] == ' ') {cmd[strlen(cmd)-1] = '\0';}
    
    
    
    if (Disable_PSH_Commands == false)
    {
        if (strcmp(cmd,"hello") == 0) {
            Command_is_Founded = true;
            hello();
        }
        else if (strncmp(cmd, "future ", 7) == 0) {
            Command_is_Founded = true;
            
            future(CmdSource, mem);
            free(CmdSource);
            return 1;
        }
        else if (strcmp(cmd, "ver") == 0) { // Show current varsion of ProggShell with ascii art
            Command_is_Founded = true;
            ver();
        }
        else if (strncmp(cmd, "delay ", 6) == 0) {
            Command_is_Founded = true;
            delay(cmd); // Wait some time
        }
        else if (strncmp(cmd, "set-lng ",8) == 0) {
            Command_is_Founded = true;
            setlng(mem->data,CmdSource); // set default language and write it to PSH_settings
        }
        else if (strncmp(cmd,"back ",5) == 0){ //Clear cmd history
            Command_is_Founded = true;
            back(CmdSource,mem);
        }
        else if (strncmp(cmd, "help",4) == 0) {
            Command_is_Founded = true;
            help(); // Show all commands from language file
        }
        else if (strncmp(cmd, "if ", 3) == 0) {
            Command_is_Founded = true;
            ifFunct(CmdSource); // execute logical if
        }
        else if (strncmp(cmd,"histexport ",11)== 0){ //Export cmd history to file
            Command_is_Founded = true;
            export_file(CmdSource,mem);
        }
        else if (strcmp(cmd, "clv") == 0) { //clear view
            Command_is_Founded = true;
            clv();
        }
        else if (strcmp(cmd,"clh") == 0){ //Clear cmd history
            Command_is_Founded = true;
            clear_history();
        }
    }
    
    if (strncmp(cmd, "cd ", 3) == 0) {
        Command_is_Founded = true;
        cd(CmdSource,mem); // call enter to folder function with CmdSource and shell memory as arguments
    }
    else if (strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0 ) {
        free(CmdSource); // free CmdSource
        return 0; // End Session
    }
    else if (strncmp(cmd, "var ",4) == 0){
        
        Command_is_Founded = true;
        create_var(CmdSource,&Variables_List,&Variables_Count);
        
    }
    else if (strncmp(cmd, "rvar ",5) == 0){
        
        Command_is_Founded = true;
        remove_var(CmdSource,&Variables_List,&Variables_Count);
        
    }

    

    else if (strcmp(cmd, "nocommands") == 0){
        Command_is_Founded = true;
        printf("Progg Shell comands disabled! To enable it enter 'excommands'\n");
        Disable_PSH_Commands = true;
    }
    else if (strcmp(cmd, "excommands") == 0){
        Command_is_Founded = true;
        Disable_PSH_Commands = false;
    }
    
    else if (strcmp(cmd, "varlist") == 0){
        Command_is_Founded = true;
        List_Variables(Variables_List,Variables_Count);
    }   
    
    #ifdef _WIN32
    else if (strncmp(cmd, "cls",3) == 0) {
        Command_is_Founded = true;
        cls(); // call clear screen function (OS addiction)
    }
    else if (strncmp(cmd, "mdr ", 4) == 0) {
        Command_is_Founded = true;
        mdr(CmdSource, mem); //call make dir function with CmdSource and mem as arguments
    }
    else if (strncmp(cmd, "crt ", 4) == 0) {
        Command_is_Founded = true;
        crt(CmdSource, mem); //call file create function with CmdSource and mem as arguments
    }
    else if (strncmp(cmd, "cop ", 4) == 0 || strncmp(cmd, "cln ", 4) == 0) {
        Command_is_Founded = true;
        copy(CmdSource, mem); // call copy function with CmdSource and mem as arguments
    }
    else if (strncmp(cmd, "mov ", 4) == 0) {
        Command_is_Founded = true;
        mov(CmdSource, mem); // call move function with CmdSource and mem as arguments
    }
    else if (strncmp(cmd, "fl", 4) == 0||strncmp(cmd, "dir", 4) == 0||strncmp(cmd, "ls", 4) == 0) {
        Command_is_Founded = true;
        dir(mem); // call dir function with mem as argument
    }
    else if (strncmp(cmd, "ren ", 3) == 0) {
        Command_is_Founded = true;
        ren(CmdSource, mem); // call raname function wirh CmdSource and shell memory as arguments
    }
    else if (strncmp(cmd, "del ", 4) == 0) {
        Command_is_Founded = true;
        del(CmdSource, mem); //Call delete function with source input and current path
    }
    else if (strncmp(cmd, "read ", 5) == 0) {
        Command_is_Founded = true;
        prtread(CmdSource); // Read file and print 
        
    }
    else if (strncmp(cmd, "prtread ", 8) == 0) {
        Command_is_Founded = true;
        prtread(CmdSource); // Read file and print with Words and Sumbols check
        
    }
    else if (strncmp(cmd, "prt ", 4) == 0) {
        Command_is_Founded = true;
        prt(CmdSource); // print argument
    }
    else if (strncmp(cmd, "run ", 4) == 0) {
        Command_is_Founded = true;
        run(CmdSource); //Ask system do a argument
    }
    else if (strncmp(cmd,"findf ",6) == 0){ //Find first
        Command_is_Founded = true;
        find(CmdSource, mem,1); // Find first match and print it
    }
    else if (strncmp(cmd,"find ",5) == 0){ //Clear cmd history
        Command_is_Founded = true;
        find(CmdSource, mem,0);
    }
    else if (strncmp(cmd,"inf ",4) == 0){ //Clear cmd history
        Command_is_Founded = true;
        FileInfo(CmdSource, mem);
    }
    else if (strncmp(cmd,"solve ",6) == 0){ //Clear cmd history
        Command_is_Founded = true;
        solve(CmdSource);
    }
    #endif
    // else if (strncmp(cmd, "exec", 4) == 0) {
    //     execCommand(CmdSourcedkaspd,mem); // Call Script execute function
    // }
    
    
    else if (Command_is_Founded == false){

        // pid_t pid = fork();
        // char* path = Get_Command_Path_From_PATHS_Massive(cmd, Commands_Path_Massive,Commands_Count, Commands_List);
        // execl(path, "")

        system(CmdSource);

        free(CmdSource);
        return 1; // continue session
    }
    
    
    free(CmdSource); //Clean up after running
    return 1; // Continue session

}



int main(int argc, char *argv[]) {




    if (AddPATHCommandstoCommandsCompletion(&Commands_List, &Commands_Path_Massive, &Commands_Count) != 0){
        
        write(1, "Failed to add commands from PATH\n",34);

        if (!yesorno(NULL)){
            return -4;
        };

    }



    futureexecute = 0;
    futurecommand.Command = NULL;
    futurecommand.CommandsCountToExecute = 0;
    ShellMemory current_path; // Path System
    init_start_path(&current_path); // Creating dynamic buffer with current path
    
    ProgrammDir = strdup(current_path.data);
    
    if (ProgrammDir == NULL){
        free(current_path.data);
        error("Memory allocation error!\n");
        return -1;
    }
    Settings = CombinePath(ProgrammDir,"PSH_settings");

    if (access(Settings,R_OK) != 0){
        error("Failed to get read access to PSH_settings");
        if (!yesorno("Continue? (Some functions may not work or crash)")){
            free(ProgrammDir);
            free(Settings);
            return -2;
        }
    }

    
    #ifdef OS_LINUX
        setlocale(LC_ALL, "");  
    #endif
    

    #ifdef _WIN32 // Windows UTF-8 support
        setlocale(LC_ALL, ".UTF8");
        setlocale(LC_NUMERIC, "C");
        SetConsoleCP(CP_UTF8);
        SetConsoleOutputCP(CP_UTF8);
        
        
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    #endif

    userinf UserInfParsed = OpenUserinf();
    if (UserInfParsed.Lng == NULL){
        error("Failed to read access to PSH_settings");
        if (!yesorno("Continue? (Some functions may not work or crash)")){
            free(ProgrammDir);
            free(Settings);
            return -2;
        }
    }
    else{
        
        MaxCommandsInHistory     = UserInfParsed.MaxCommandsInHistory;
        langFile=UserInfParsed.Lng; //Get language path from user-inf.json

    }
    
    

    if (MaxCommandsInHistory <= 0) MaxCommandsInHistory = 100;
    //stifle_history(MaxCommandsInHistory); ------------------------------------------------------DONT FORGET FIX!


    psh_hist_entries = calloc(MaxCommandsInHistory, sizeof(char*));
    if (!psh_hist_entries) {
        error("Critical Memory allocation error!\n");
        return -3;
    }
    //StringVariable* vars = NULL;
    
    // if (argc > 1 && strncmp(argv[1], "debug", 5) != 0) { // If Progg Shell started with argument (Script path) Progg shell trying to execute it
    //     print_s("Loading: %s\n", argv[1]);
    //     __int8_t Sccode = 0;
        
    //     FILE *file = fopen(argv[1], "r");
    //     if (file) {
    //         write(1,"Loaded file!\n",13);
    //         fclose(file);
    //         Sccode = 0;
    //     } else {
    //         Sccode = 1;
    //         error("Cant open file!");
    //     }   
        
    //     write(1,"Script Ended with code ",23);
    //     print_int(Sccode);
    //     write(1,"\n",1);
    //     free(current_path.data);
    //     return(Sccode);
    // }

    #ifdef _WIN32
        if (!SetConsoleTitle("Progg Shell")){warn("Failed rename console!");}
    #endif
    
    rl_attempted_completion_function = psh_completion;
    
    while (1) {
        
        write(1,"\n",1);
        DynamicBuffer db_prompt;
        init_buffer(&db_prompt, 64); 
        
        append_to_buffer(&db_prompt, current_path.data, strlen(current_path.data)); 
        append_to_buffer(&db_prompt, " > ", 3);

   
        char *input = readline(db_prompt.data);

        free_buffer(&db_prompt); 
        if (!input) {
            error("Memory allocation error!");
            break;
        }

       
        if (input && input[0] != '\0') {
            
            add_history(input);
            int slot = psh_hist_head % MaxCommandsInHistory;
            free(psh_hist_entries[slot]);                 
            psh_hist_entries[slot] = strdup(input);         
            psh_hist_head++;
            if (psh_hist_count < MaxCommandsInHistory){
                psh_hist_count++;
            }

            if (handleCommand(input, &current_path) == 0) {
                free(input);
                break;
            }
            else{
                if (futureexecute > 0){
                    if (futurecommand.CommandsCountToExecute < 1){
                        handleCommand(futurecommand.Command,futurecommand.Path);
                        futureexecute --;
                    }
                    else{
                        futurecommand.CommandsCountToExecute --;
                    }
                }
            }
        }
        free(input);
    }


    for (int i = 0; i < MaxCommandsInHistory; i++){
        free(psh_hist_entries[i]);
    }

    free(psh_hist_entries);
    free(langFile); // Clear after session
    free(current_path.data);
    free(ProgrammDir);
    free(Settings);


    
    for (size_t i = 0; i < Commands_Count; i++){
        free(Commands_List[i].Name);
    }
    
    free(Commands_Path_Massive);
    free(Commands_List);

    for (size_t i = 0; i < Variables_Count; i++){
        free(Variables_List[i].name);
        free(Variables_List[i].data);
    }
    


    clear_history(); //Need be deleted?
    



    return 0;
}
