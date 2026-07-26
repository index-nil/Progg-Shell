

/*--------------------------------File(and other) Utils Module--------------------------------*/




//Progg Shell 2026.01.12 File module MIT 
//DONT FORGOT ADD FREE()!!!

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fum.h"
#include <sys/stat.h>
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #define NOUSER
    #define NOCRYPT

    #include <windows.h>
    size_t count_symbols(const char* buffer, size_t length) {
        if (length == 0 || !buffer) return 0;

        size_t count = 0;
        for (size_t i = 0; i < length; i++) {
            if (buffer[i] != '\r' && buffer[i] != '\t' &&
                buffer[i] != '\n' && buffer[i] != '\0') {
                count++;
            }
        }
        return count;
    }
    
    size_t count_words(const char* buffer,  size_t length) {
        if (length <= 0 || !buffer) return 0;
    
        long words = 0;
        int in_word = 0;
    
        for (long i = 0; i < length; i++) {
            if (buffer[i] != ' ' && buffer[i] != '\n' && buffer[i] != '\r' && buffer[i] != '\t') {
                if (!in_word) {
                    words++;
                    in_word = 1;
                }
            } else {
                in_word = 0;
            }
        }
        return words;
    }



#endif

// //idk why it here
// char* TakeFromString(const char* string, const char* word){ //Return pointer to memory with

    
int is_directory(const char *path) {
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(path); 

    return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
           (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return 0; 
    }
    return S_ISDIR(statbuf.st_mode);
#endif
}
// }



char* CombinePath(const char *folder, const char *file) {
    if (!folder || !file) return NULL;


    size_t len = strlen(folder) + strlen(file) + 2; 
    
    char *newPath = (char*)malloc(len);
    if (!newPath){
        return NULL;
    } 

    

    snprintf(newPath, len, "%s/%s", folder, file);
    
    return newPath;
}

char* CombineStrings(const char* str_one, const char* str_two){
    
    size_t str_one_len = strlen(str_one);
    size_t str_two_len = strlen(str_two);

    char* buf = malloc(str_one_len + str_two_len + 1);
    if (buf == NULL) {
        return NULL;
    }

    size_t i;

    for (i = 0; i < str_one_len; i++){
        buf[i] = str_one[i];
    }

    for (i = 0; i < str_two_len; i++){
        buf[str_one_len + i] = str_two[i];
    }

    buf[str_one_len + str_two_len] = '\0';

    return buf;
}

char* str_replace_all(const char* str, const char* find, const char* replace){
    char* result = strdup(str);
    if(result == NULL) return NULL;

    size_t find_len = strlen(find);
    size_t replace_len = strlen(replace);
    size_t search_offset = 0;

    if(find_len == 0) return result;

    while (1) {
        char* found = strstr(result + search_offset, find);
        if (found == NULL) break;

        size_t prefix_len = found - result;
        size_t suffix_len = strlen(found + find_len);

        char* new_buf = malloc(prefix_len + replace_len + suffix_len + 1);
        if (new_buf == NULL) {
            free(result);
            return NULL;
        }

        memcpy(new_buf, result, prefix_len);
        memcpy(new_buf + prefix_len, replace, replace_len);
        memcpy(new_buf + prefix_len + replace_len, found + find_len, suffix_len + 1);

        free(result);
        result = new_buf;

        search_offset = prefix_len + replace_len;
    }

    return result;
}