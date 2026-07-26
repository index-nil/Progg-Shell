


#define _DEFAULT_SOURCE
#define _FILE_OFFSET_BITS 64

#include "Commads.h"
#include "types.h"
#include "fum.h" //File Utils Module 
// #include "UIlib.h" //Progress bar, graph and other
#include "../main.h"
#include <stdio.h>
#include <errno.h>
#include "LngModule.h" //Localization
#include <string.h> //String functions
#include <stdlib.h>  // System control
#include <errno.h> //Error number print 
#include "print.h" //Write
#include "UIlib.h"
// #include <editline/readline.h>
// #include <stddef.h>
#include <time.h>

#ifdef _WIN32

    #include <sys/stat.h>
    #include <windows.h>    
    #include <dirent.h> // dir || fl commands
    #include <direct.h>
    #define osPathSeparateSymbol ';'
    #define GetCurrentDir _getcwd
    #define ChangeDir _chdir
    #define WIN32_LEAN_AND_MEAN
    #define SLEEP_MS(x) Sleep(x)
    
    
#else
    #include <sys/types.h>
    #include <unistd.h>
    #include <sys/ioctl.h> // window size

    #define osPathSeparateSymbol ':'
    #define GetCurrentDir getcwd
    #define ChangeDir chdir
    #define SLEEP_MS(x) usleep((x)*1000)
    #define OS_LINUX
    
#endif



#ifdef _WIN32

int MoveFileFunct (char* filepatch, char* newpath){ //Need Be deleted
    if (filepatch == NULL || newpath == NULL) {
        write(1,"Invalid file path(s) provided.\n",31);
        return 1;
    }
    if (rename(filepatch, newpath) == 0) {
        write(1,"File moved successfully.\n",25);
        return 0;
    } else {
        print_s("Error moving file: %s\n", strerror(errno));
        return 1;
    }
    
}






void cls(){

    system("cls");
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
        write(1,"Usage: mov <source> <destination>\n",34);
        return 1;
    }


    char *arg2 = arg;
    while (*arg2 != ' ' && *arg2 != '\0') arg2++;
    if (*arg2 == '\0') {
        write(1,"Usage: mov <source> <destination>\n",34);
        return 1;
    }

    *arg2 = '\0';
    char *new_name = arg2 + 1;
    while (*new_name == ' ') new_name++;
    if (*new_name == '\0') {
        write(1,"Usage: mov <source> <destination>\n",34);
        return 1;
    }


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


    const char *src_basename = strrchr(arg, '/');
    const char *bslash = strrchr(arg, '\\');
    if (bslash && (!src_basename || bslash > src_basename)) src_basename = bslash;
    if (src_basename) src_basename++; else src_basename = arg;

    char *dst_path = NULL;

    if (new_name[0] == '/' || new_name[0] == '\\' || (isalpha((unsigned char)new_name[0]) && new_name[1] == ':')) {
        if (is_directory(new_name)) {
            size_t s = strlen(new_name) + strlen(src_basename) + 2;
            dst_path = malloc(s);
            if (dst_path) {
                char *p = dst_path;
                

                const char *s1 = new_name;
                while (*s1) *p++ = *s1++;
                

                *p++ = '/';

                const char *s2 = src_basename;
                while (*s2) *p++ = *s2++;

                *p = '\0';
            }
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
            if (dst_path && s > 0) {
                char *d = dst_path;
                char *end = dst_path + s - 1; 
                const char *p1 = candidate;
                while (*p1 && d < end) *d++ = *p1++;


                if (d < end) *d++ = '/';


                const char *p2 = src_basename;
                while (*p2 && d < end) *d++ = *p2++;

                *d = '\0';
            }
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

    if (MoveFileFunct(old_path, dst_path) == 0) {
        done();
    }

    free(old_path);
    free(dst_path);
    return 0;
}

void crt(char* CmdSource, ShellMemory* mem){
    char *arg = CmdSource + 4; 
    while (*arg == ' ') arg++; 
    char* FullPath = CombinePath(mem->data, arg);
    if (*arg != '\0') {
        FILE *file = fopen(FullPath,"w"); 
        if (file){
            fclose(file);
            done();
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
        write(1,"No command to execute!\n",23);
    }
     
    else {
        int Status = system(arg);
        if (Status == 0) {
            done();
        }
        else {
        
            write(1, "\033[38;2;209;0;0m[Error]: executing command '", 42);

            if (arg) {
                write(1, arg, strlen(arg));
            } else {
                write(1, "NULL", 4);
            }

            write(1, "' Error code: ", 14);


            print_int(Status);


            write(1, "\033[0m\n\n", 6);
        }
    }
}
int copy(char* CmdSource, ShellMemory* mem) {
    /* Usage: cop <source> <destination>
           Supports: cop(copy), cln (clone) */
        int cmd_len = 4;

        char *arg = CmdSource + cmd_len;
        while (*arg == ' ') arg++;

        if (*arg == '\0') {
            write(1,"Usage: cop <source> <destination>\n",34);
            
            return -1;
        }

        /* split into source and destination (simple, like other commands) */
        char *sep = arg;
        while (*sep != ' ' && *sep != '\0') sep++;
        if (*sep == '\0') {
            write(1,"Usage: cop <source> <destination>\n",34);
            
            return -2;
        }
        *sep = '\0';
        char *dst = sep + 1;
        while (*dst == ' ') dst++;
        if (*dst == '\0') {
            write(1,"Usage: cop <source> <destination>\n",34);
            
            return -3;
        }
        
        char *srcPath = CombinePath(mem->data, arg);
        char *dstPath = CombinePath(mem->data, dst);
        
        if (strcmp(srcPath, dstPath) == 0) {
            error("Source and destination are the same!");
            
            free(srcPath); free(dstPath); 
            return -4;
        }

        FILE *in = fopen(srcPath, "rb");
        if (!in) {
            write(1, "\033[38;2;209;0;0mError opening '", 30);

            if (srcPath) write(1, srcPath, strlen(srcPath));

            write(1, "': ", 3);

            char* err_msg = strerror(errno); 
            write(1, err_msg, strlen(err_msg));

            write(1, "\033[0m\n", 5);
            free(srcPath); free(dstPath); 
            return -5;
        }

        FILE *out = fopen(dstPath, "wb");
        if (!out) {

            write(1, "\033[38;2;209;0;0mError opening '", 30);


            if (srcPath) write(1, srcPath, strlen(srcPath));

            write(1, "': ", 3);

            char* err_msg = strerror(errno); 
            write(1, err_msg, strlen(err_msg));

            write(1, "\033[0m\n", 5);
            fclose(in); free(srcPath); free(dstPath);
            return -6;
        }
        DynamicBuffer buf;
        /* use a larger buffer and read into buf.capacity (not buf.size) */
        init_buffer(&buf,2048);
        size_t n;
        int copy_error = 0;
        while ((n = fread(buf.data, 1, buf.capacity, in)) > 0) {
            size_t written = fwrite(buf.data, 1, n, out);
            if (written != n) {
                print_s("\033[38;2;209;0;0mWrite error: %s\033[0m\n", strerror(errno));
                copy_error = 1;
                break;
            }
        }
        if (ferror(in)) {
            print_s("\033[38;2;209;0;0mRead error: %s\033[0m\n", strerror(errno));
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
        return 0;
}

void mdr(char* CmdSource, ShellMemory* mem) {
    char *arg = CmdSource + 4; 
    while (*arg == ' ') arg++; 
    char* FullPath = CombinePath(mem->data, arg);
    if (*arg != '\0') {
        if (_mkdir(FullPath) != 0){print_s("\033[38;2;209;0;0mError: %s\033[0m\n",strerror(errno));}
        else{write(1,"\033[38;2;33;198;0mDone\033[0m\n",25);}
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
        // int Folderlenght = strlen(folder_name);
        print_s("\n|───────[ %s fill ]───────|\n",folder_name);
        while ((entry = readdir(dp)) != NULL) { //Need be optimizated
            
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            char* full_path = CombinePath(mem->data, entry->d_name);
            count++;
            
            if (is_directory(full_path)){
                print_s("\033[38;2;0;0;215m[DIR]\033[0m %s",entry->d_name);
                write(1,"\n",1);
                
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
                
                print_s("[FILE] %s",entry->d_name);
                write(1,"\n",1);
            }
            free(full_path);
        }
        // char buf[Folderlenght + 1];
        // memset(buf,'-',Folderlenght +1);
        // // buf[Folderlenght + 2] = '\0';
        // write(1,"\n|───────",17);
        // write(1,buf,Folderlenght + 1);
        print_int(count);
        // write(1,"---------\n",10);
        closedir(dp);
    }
    else {
        print_s("\033[38;2;209;0;0mError opening directory: %s\033[0m\n", strerror(errno));
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
            write(1,"\033[38;2;33;198;0mDone\033[0m\n",25);
        } else {
            print_s("\033[38;2;209;0;0mError: %s\033[0m\n",strerror(errno));
        }
        
        free(old_path);
        free(new_path);
    }
    else {
        write(1,"Usage: ren <old_name> <new_name>\n",33);
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

                append_to_buffer(&FullPath, "\\", 1);
            }
            append_to_buffer(&FullPath, arg, strlen(arg));
            if (remove(FullPath.data) != 0) {
                error(strerror(errno));
            } else {
                write(1,"\033[38;2;33;198;0mFile successfully deleted!\033[0m\n",47);
            }
            free_buffer(&FullPath);
        }
        else{
            write(1,"Operation Aborted\n",18);
        }
        return;
    }
}
void prt(char* CmdSource){
    char *arg = CmdSource + 4;
    while (*arg == ' ') arg++;
    if (*arg == '\0'){
        write(1,"\n",1);
    }
    else {
        write(1,arg,strlen(arg));
        write(1,"\n",1);
    }
        
}
void prtread(char* CmdSource) {

    char *arg = CmdSource + 8;
    while (*arg == ' ') arg++;

    if (*arg == '\0') {
        char *msg = getSection("read:fileNameError", langFile,0);
        if (msg) {
            write(1, msg, strlen(msg));
            write(1, "\n", 1);
            free(msg);
        }
        return;
    }

    long filesize = 0;
    char *content = NULL;
    
    FILE *file = fopen(arg, "rb");
    
    char *buf;
    
    if (file) {
        fseek(file, 0, SEEK_END);
        long length = ftell(file);
        fseek(file, 0, SEEK_SET);
        char *buf = malloc(length + 1);
        if (!buf) {
            error("Memory allocation error!");
            fclose(file);

        }
        
        fread(buf, 1, length, file);
        buf[length] = '\0';
        filesize = length;
        content = buf;
        fclose(file);
            
    }
    

    if (!content) {
        char *msg = getSection("general:cantOpenFile", langFile,0);
        if (msg) {
            
            write(1, msg, strlen(msg));
            write(1, "\n", 1);
            free(msg);
            
        } else {
            error("Memory allocation error! cantOpenFile");
        }
        return;
    }

    write(1, content, strlen(content));
    write(1, "\n\n\n", 3);

    size_t words   = count_words(content, filesize);
    size_t symbols = count_symbols(content, filesize);
    double readtime = Readtime;
    readtime = words / readtime;

    free(content);
    
    printf("Words in file: %lu | Symbols: %lu | Readtime for you: %lf~\n", words, symbols, readtime);

}

//StringVariable* vars

// void execCommand(char* CmdSource,ShellMemory* mem){
//     char *arg = CmdSource + 4;
//     while (*arg == ' ') arg++;
//     if (*arg == '\0'){
//         error("Usage <path_to_file>\n");
//     }
    
//     else {
//         exec(arg,mem);
//     }
// }





//Deleted ReadUserInfLanguage, ReadUserinfReadtime, ReadUserinfMaxCommandsInHistory and other for optimization



 





int find(char* CmdSource, ShellMemory* mem,int mode){ //Find all matches in folder need be optimizated
    char* arg;
    if (mode == 0){
        arg = CmdSource + 5;
    }
    else {
        arg = CmdSource + 6;
    }
    while (*arg == ' ') arg++;
    size_t len = strlen(arg);
    while (len > 0 && (arg[len - 1] == '\n' || arg[len - 1] == '\r')) {
        arg[--len] = '\0';
    }

    if (len == 0) return 0; 
    struct dirent *entry;
    DIR *dp = opendir(mem->data);

    if (dp == NULL){
        return -1;        
    }
    while ((entry = readdir(dp))) {    
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if (mode == 0){
            if (strncmp(entry->d_name,arg,len) == 0 ) { 
                write(1, entry->d_name, strlen(entry->d_name));
                write(1, "\n", 1);
                
            }
        }
        else{
            if (strncmp(entry->d_name,arg,len) == 0) { 
                write(1, entry->d_name, strlen(entry->d_name));
                write(1, "\n", 1);
                break;
            }
        }
    }
    
    while ((entry = readdir(dp))) {    
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        for (size_t i = 0; i < len; i++)
        {
            if (mode == 0){
                
            }
        }
        
    }

    closedir(dp);
    return 0;
}

void shutdownComputer() {
    if (!system("shutdown -h now")){(void)write(1,"Failed to shutdown computer! Please try again\n",46);};            
    exit(0);
}

void FileInfo(char* CmdSource, ShellMemory* mem){
    struct stat file_info;
    char* arg = CmdSource + 4;
    while (*arg == ' '){arg++;}
    if (*arg == '\0'){
        char* sys_error = strerror(errno);
        error(sys_error);
        return;
    }
    
    char* PathToFile = CombinePath(mem->data, arg);

    if (stat(PathToFile,&file_info) != 0){
        char* sys_error = strerror(errno);
        error(sys_error);
        free(PathToFile);
        return;
    }

    print_s("\n\033[38;2;8;0;232m[Info of ]: %s\033[0m\n", arg);
    write(1,"--------------------------\n",27);


    write(1, "\nSize: ", 7);
    print_int((long)file_info.st_size);
    write(1, "B\n", 2);

    DWORD highOrder;
    DWORD lowOrder = GetCompressedFileSize(PathToFile, &highOrder);
    unsigned long long actualSize = ((unsigned long long)highOrder << 32) | lowOrder;
    write(1, "Actual size on disk: ", 21);
    printf("%lu",actualSize);
    write(1, " bytes\n", 7);

    write(1, "On Disk: ", 9);
    print_int((long)file_info.st_dev);
    write(1, "\nCreator: ", 10);
    print_int((long)file_info.st_uid);
    write(1, "\n", 1);

    char* time_str;
    write(1, "Last edit time: ", 16);
    time_str = ctime(&file_info.st_mtime);
    if (time_str) write(1, time_str, strlen(time_str));

    write(1, "Last access time: ", 18);
    time_str = ctime(&file_info.st_atime);
    if (time_str) write(1, time_str, strlen(time_str));

    write(1, "Permissions: ", 13);
    write(1, (file_info.st_mode & S_IRUSR) ? "r" : "-", 1);
    write(1, (file_info.st_mode & S_IWUSR) ? "w" : "-", 1);
    write(1, (file_info.st_mode & S_IXUSR) ? "x" : "-", 1);
    write(1, "\n", 1);

    write(1, "Type: ", 6);
    if (S_ISDIR(file_info.st_mode)) {
        write(1, "DIR\n", 4);
    } else {
        write(1, "FILE\n", 5);
    }

    free(PathToFile);
    return;

}

void solve(char* CmdSource){ //Buggy
    if (!CmdSource){
        return;
    }
    
    char* arg1 = CmdSource + 6;
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

    long num1 = atoi(arg1);
    long num3 = atoi(arg3);
    long answ = 0;
    
    if (*arg2 == '+'){
        answ = num1 + num3;
        print_long(answ);
        write(1,"\n",1);
    }
    else if (*arg2 == '-'){
        answ = num1 - num3;
        print_long(answ);
        write(1,"\n",1);
    }
    else if (*arg2 == '/'){
        if (num3 == 0){
            write(1, "Error: division by zero\n", 24);
            return;
        }
        answ = num1 / num3;
        print_long(answ);
        write(1,"\n",1);
    }
    else if (*arg2 == '*'){
        answ = num1 * num3;
        print_long(answ);
        write(1,"\n",1);
    }
    else{
        write(1,"Usage: solve <num1> <+,-,/,*> <num2>\n",37);
    }
    
}

#endif





void strToLower(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}




/*------------------Basic shell commands starts here------------------*/



// void forLoop(char* CmdSource){
//     if (!CmdSource){
//         return;
//     }
    
//     char* arg1 = CmdSource + 6;
//     while (*arg1 == ' ') arg1++; 
//     char* end_arg1 = arg1;
//     while (*end_arg1 != ' ' && *end_arg1 != '\0') end_arg1++;
//     char original_char1 = *end_arg1; 
//     *end_arg1 = '\0'; 
//     char* arg2 = end_arg1 + 1;
//     if (original_char1 != '\0') {
//         while (*arg2 == ' ') arg2++;
//     }
//     char* end_arg2 = arg2;
//     while (*end_arg2 != ' ' && *end_arg2 != '\0') end_arg2++;
//     char original_char2 = *end_arg2;
//     *end_arg2 = '\0'; 
//     char* arg3 = end_arg2 + 1;
//     if (original_char2 != '\0') {
//         while (*arg3 == ' ') arg3++;
//     }

//     arg1 = atoi(arg1);
//     arg3 = atoi(arg3);

//     // DynamicBuffer db;
//     // init_buffer(&db,64);


//     if (*arg2 == '>'){
//         for ( arg1 > arg3; arg1++;){
//             while
//         }
        
//     }

// }

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
            else{
                error("Failed to expand ShellMemory\n");
                free_buffer(&temp);
                return;
            }
        }

        strcpy(mem->data, temp.data);

        free_buffer(&temp);

    } else {

        write(2, "Error changing dir to '", 23);


        if (target) {
            write(2, target, strlen(target));
        } else {
            write(2, "NULL", 4);
        }


        write(2, "': ", 3);


        char* err_msg = strerror(errno);
        if (err_msg) {
            write(2, err_msg, strlen(err_msg));
        } else {
            write(2, "Unknown error", 13);
        }


        write(2, "\n", 1);
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

void hello(){
    write(1,"Hello World!\n",13);
}





int back(char* CmdSource,ShellMemory* mem){
    if (!mem || !mem->data) return -1;
    char* arg = CmdSource + 5;
    while (*arg == ' ') arg++;
    int len = strlen(mem->data);
    int loop_iterations = atoi(arg);
    if (loop_iterations <=0){
        error("Failed converting string to number! (Are you entered a Number?)\n");
        return -1;
    }
    
    if (len > 1){
        len -= 1;
    }
    else{
        error("You already in root directory!\n");
        return -1;
    }
    char* buf = strdup(mem->data);
    if (!buf) return -1;
    for (size_t i = 0; i < loop_iterations; i++)
    {
        
        if (len > 0 && (buf[len] == '/' || buf[len] == '\\')) {
            len--;
        }
        while ( len > 0 && buf[len] != '\\'  && buf[len] != '/' ){
            len --;
        } 
        if (len <= 1) break;
    }
        
    if (len < 0) {
        warn("To many backs! returning to root directory\n");
        len = 0;
    }
    #ifdef _WIN32
        buf[len] = '\\' ;
    #else
        buf[len] = '/' ;
    #endif
    buf[len + 1] = '\0';
    psh_change_dir(buf,mem);
    free(buf);
    return 0;
    
}

int cd(char* CmdSource, ShellMemory* mem) {
    char *path_arg = CmdSource + 3; 
    while (*path_arg == ' ') path_arg++; 
    
    if (*path_arg != '\0') {
        psh_change_dir(path_arg, mem); 
    }
    
    return 0;
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
        write(1, (strcmp(arg1, arg3) == 0 ? "True\n" : "False\n"), (strcmp(arg1, arg3) == 0 ? 5 : 6));
    } 
    else if (strcmp(arg2, "!=") == 0) {
        write(1, (strcmp(arg1, arg3) != 0 ? "True\n" : "False\n"), (strcmp(arg1, arg3) != 0 ? 5 : 6));
    } 
    else if (strcmp(arg2, ">") == 0) {
        int val = (atoi(arg1) > atoi(arg3));
        write(1, (val ? "True\n" : "False\n"), (val ? 5 : 6));
    } 
    else if (strcmp(arg2, "<") == 0) {
        int val = (atoi(arg1) < atoi(arg3));
        write(1, (val ? "True\n" : "False\n"), (val ? 5 : 6));
    }
    else {
        write(1,"Only '==', '!=', '>', '<' operators supported\n",46);
    }
                
    *end_arg1 = original_char1;
    *end_arg2 = original_char2;
    
}


int8_t export_file(char* CmdSource, ShellMemory* mem) { 
    char* arg = CmdSource + 7;
    while (*arg == ' ') arg++;

    if (!*arg) {
        write(1, "Usage: export <filename>\n", 25);
        return -3;
    }

    if (!psh_hist_entries || MaxCommandsInHistory <= 0) {
        write(1, "History not available.\n", 23);
        return -3;
    }

    if (psh_hist_count == 0) {
        write(1, "History is empty.\n", 18);
        return -3;
    }

    char* Path = CombinePath(mem->data, arg);
    if (!Path) return -3;

    if (access(Path, F_OK) == 0) {
        print_s("[Warning]: In this folder already exist '%s' ", arg);
        if (yesorno("replace?") == 0) {
            write(1, "Operation Aborted\n", 18);
            free(Path);
            return -1;
        }
    }

    FILE* f = fopen(Path, "w");
    if (!f) {
        perror("Export failed");
        free(Path);
        return -2;
    }

    for (int i = 0; i < psh_hist_count; i++) {
        int slot = (psh_hist_head - psh_hist_count + i + MaxCommandsInHistory)
                   % MaxCommandsInHistory;
        if (psh_hist_entries[slot])
            fprintf(f, "%s\n", psh_hist_entries[slot]);
    }

    if (fclose(f) != 0) {
        perror("Export failed (fclose)");
        free(Path);
        return -2;
    }

    print_s("History exported to '%s' successfully.\n", arg);
    free(Path);
    return 0;
}




void future(char * CmdSource, ShellMemory* mem) {

    char* ptr = CmdSource;
    while (*ptr != ' ' && *ptr != '\0') ptr++; 

    while (*ptr == ' ') ptr++;
    
    if (*ptr == '\0') {
        error("Usage: future <count> <command>");
        return;
    }


    futurecommand.CommandsCountToExecute = atoi(ptr);


    while (*ptr != ' ' && *ptr != '\0' && (*ptr >= '0' && *ptr <= '9')) ptr++;
    

    while (*ptr == ' ') ptr++;

    if (*ptr == '\0') {
        error("No command specified!");
        return;
    }

    if (futurecommand.Command != NULL) free(futurecommand.Command);
    futurecommand.Command = strdup(ptr); 
    futurecommand.Path = mem;
    futureexecute = 1; 
    
    write(1, "Task scheduled: '", 17);


    if (ptr) {
        write(1, ptr, strlen(ptr));
    } else {
        write(1, "NULL", 4);
    }


    printf("' in %d commands\n",futurecommand.CommandsCountToExecute);
}




int8_t setlng(const char* Path ,char * CmdSource) {
    int8_t exCode = 0;
    char *arg = CmdSource + 7;
    while (*arg == ' ') arg++;
    
    if (strlen(arg) == 0) {
        error("Specify language file!");
        return -1;
    }

    char* tempLang = CombinePath(Path, arg);
    if (!tempLang) {
        perror("Memory allocation error");
        error("Memory allocation error! tempLang section");
        return -1;
    }

    if (access(tempLang, 0) != 0) {
        error("Language file not found!");
        free(tempLang);
        return -1;
    }


    if (WriteToSection("Language",tempLang,Settings) != 0){
        error("Failed to write language to PSH_settings! Accepting temporary changes. . .");
        free(langFile);
        langFile = strdup(tempLang);
        if (langFile == NULL){
            error("Memory allocation error!");
        }
        exCode = -1;
    }
    

    free(tempLang);
    return exCode;
}

void ver() {
 
    const char logo[] = 
        "\n⠀⠀⣠⡿⠉⠉⢻⣇⠀⠀⠀⠀⠀⣴⠟⠉⠉⢿⡆⠀⠀⠀⠀⢠⡾⠋⠉⠹⣷⠀"
        "\n⠀⢠⣿⠀⠀⠀⢸⡿⠀⠀⠀⠀⢸⡏⠀⠀⠀⣸⡇⠀⠀⠀⠀⣾⡇⠀⠀⢠⣿⠀"
        "\n⠀⢸⡇⠀⠀⠀⣼⡇⠀⠀⠀⠀⣿⠃⠀⠀⠀⣿⠁⠀⠀⠀⠀⠘⠻⠶⠚⣻⡇⠀"
        "\n⠀⢸⣇⠀⠀⣰⡟⠀⢀⣀⠀⠀⣿⡀⠀⢀⣼⠏⠀⣀⡀⠀⠀⠀⠀⢀⣰⡟⠀⠀"
        "\n⠀⠈⠛⠛⠛⠋⠀⠀⠘⠋⠀⠀⠈⠛⠛⠛⠁⠀⠀⠛⠃⠀⠘⠛⠛⠛⠉⠀⠀⠀\n\n"
        "\033[38;2;212;0;255mProggShell v0.0.9 ";
        
    write(1, logo, sizeof(logo) - 1);
    #ifdef _WIN32
    write(1,"Windows ", 8);
    #else
    write(1,"Linux ", 6);
    #endif
    printf("| index-nil | MIT License | You can edit and share this program (With MIT License rules)\033[0m\n");
    
}


void clv(){  // Clear view
    #ifndef _WIN32
    struct winsize w;

    ioctl(0, TIOCGWINSZ, &w);

    int height = w.ws_row;

    for (size_t i = 0; i < height; i++){
       write(1,"\n",1);
    }
    #else
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    for (int i = 0; i < height; i++){
        _write(1, "\n", 1);
    }
    #endif
}

void help(){ //Pring help section from lang file
    char* Temp = getSection("help", langFile,0);
    if (Temp == NULL){
        error("The partition of language is free or does not exist");
    }
    else{
        write(1, Temp, strlen(Temp));
        free(Temp);
    }
}



void delay(char* cmd){
    char *arg = cmd + 6;
    while (*arg == ' ') arg++;
    if (*arg == '\0'){
        write(1,"No time to wait!\n",17);
    }
    
    else {
        
        long ms = atol(arg);
        SLEEP_MS(ms);
    }
}




userinf OpenUserinf(){
    userinf General = {0};
    General.Lng = strdup(getSection("Language",Settings,1)); //Not optimizated!!!
    if (General.Lng == NULL){
        error("OpenUserinf Failed to copy 'Language' section value to langfile variable!");
    }
    General.Readtime = atoi(getSection("Readtime",Settings,1));
    General.MaxCommandsInHistory = atoi(getSection("Max_Commands_In_History_File",Settings,1));
    return General;
}





int remove_var(char* cmd, Variable ** Global_Variable_List, unsigned int* Variables_Count){

    size_t i = 0;
    char* buf = malloc(strlen(cmd) + 1);

    if (buf == NULL){
        error("Memory allocation error!");
        return -1;
    }

    while (cmd[i] != '\0' && cmd[i] != ' '){
        i++;
    }


    while (cmd[i] == ' '){
        i++;
    }

    size_t a = 0;
    while (cmd[i] != '\0' && cmd[i] != ' '){
        buf[a] = cmd[i];
        a++;
        i++;
    }
    buf[a] = '\0';

    bool isExist = false;
    unsigned int var_index = 0;

    for (size_t j = 0; j < *Variables_Count; j++){
        if (strcmp((*Global_Variable_List)[j].name, buf) == 0){
            isExist = true;
            var_index = j;
            break;
        }
    }

    if (isExist != true){
        printf("'%s' not found in variables list\n", buf);
        free(buf);
        return -2;
    }


    free((*Global_Variable_List)[var_index].name);
    free((*Global_Variable_List)[var_index].data);

    for (size_t j = var_index; j < *Variables_Count - 1; j++){
        (*Global_Variable_List)[j] = (*Global_Variable_List)[j + 1];
    }

    (*Variables_Count)--;


    if (*Variables_Count > 0){
        Variable *tmp = realloc(*Global_Variable_List, (*Variables_Count) * sizeof(Variable));
        if (tmp != NULL){
            *Global_Variable_List = tmp;
        }

    }
    else{
        free(*Global_Variable_List);
        *Global_Variable_List = NULL;
    }

    free(buf);
    return 0;
}





int create_var(char* cmd, Variable** Global_Variable_List, unsigned int * Variables_Count){ //Create psh variable 

    int NameIndex = -1;
    int EqualsIndex = -1;
    int DataIndex = -1;
    
    unsigned int input_len = strlen(cmd);
    
    size_t i = 4;
    
    char* buf = malloc(input_len + 1);
    
    
    if (buf == NULL){
        error("Memory allocation error!");
        return -1;
    }
    
    memset(buf,0,input_len);
    buf[input_len] = '\0';
    




    while (cmd[i] != '\0'){
        
        if (cmd[i] != ' ' && cmd[i] != '\0' && EqualsIndex == -1 && NameIndex == -1){
            NameIndex = i;
        }
        
        
        if (cmd[i] == '='){
            EqualsIndex = i;
        }
        
        if (EqualsIndex != -1 && i > EqualsIndex && DataIndex == -1 && cmd[i] != ' '){
            
            if (cmd[i]!= '\0'){
                DataIndex = i;
            }
            
        }
        
        i++;
        
    }
    
    
    
    if (EqualsIndex == -1 || DataIndex == -1 || NameIndex == -1){
        write(1, "Usage: var (Variable_Name) = (data)\n", 36);
        free(buf);
        return -2;
    }
    
    i = NameIndex;
    
    while (cmd[i] != ' ' && cmd[i] != '\0'){
        buf[i - NameIndex] = cmd[i];
        i++;
    }
    
    if (cmd[i] == '\0'){
        free(buf);
        return -3;
    }
    
    bool isExist = false;
    unsigned int var_index = -1;

    for (size_t a = 0; a < *Variables_Count; a++){
        if (strcmp((*Global_Variable_List)[a].name, buf) == 0){
            isExist = true;
            var_index = a; 
            break;
        }
        
    }
    

    if (isExist != true){
        
        Variable * New_Var_List = realloc(*Global_Variable_List, (sizeof(Variable) * (*Variables_Count + 1)));
        if (New_Var_List == NULL){
            error("Memory allocation error!");
            free(buf);
            return -4;
        }
        *Global_Variable_List = New_Var_List;
        
        
        (*Global_Variable_List)[*Variables_Count].name = strdup(buf);
        
        if ((*Global_Variable_List)[*Variables_Count].name == NULL){
            free(buf);
            error("Memory allocation error!");
            return -5;
        }
        
    }
    
    i = DataIndex;
    
    
    memset(buf,0,input_len); //Clear buf
    buf[input_len] = '\0';
    
    while (cmd[i] != '\0'){
        buf[i - DataIndex] = cmd[i];
        i++;
    }
    

    if (var_index != -1){
        

        free((*Global_Variable_List)[var_index].data);
        (*Global_Variable_List)[var_index].data = NULL;

        (*Global_Variable_List)[var_index].data = strdup(buf);

        if ((*Global_Variable_List)[var_index].data == NULL){
            free((*Global_Variable_List)[*Variables_Count].name);
            free(buf);
            error("Memory allocation error!");
            return -6;
        }
        
        free(buf);
        return 0;

    }

    (*Global_Variable_List)[*Variables_Count].data = strdup(buf);
    
    if ((*Global_Variable_List)[*Variables_Count].data == NULL){
        free((*Global_Variable_List)[*Variables_Count].name);
        free(buf);
        error("Memory allocation error!");
        return -6;
    }
    
    free(buf);

    (*Variables_Count)++;
    
    return 0;
    
    
    
}



void List_Variables(Variable* Var_list, unsigned int Var_Count){

    printf("Existing variables:\n\n");

    for (unsigned int i = 0; i < Var_Count; i++){
        printf("%d. %s = %s\n", i + 1, Var_list[i].name, Var_list[i].data);
    }
    
    printf("\n------------------\n");

    return;
}