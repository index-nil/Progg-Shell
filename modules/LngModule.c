#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "types.h"
#include "print.h"
//Progg Shell 2026.02.1 Language Module MIT License
//DONT FORGET ADD FREE()!!!



// FILE* file = fopen(langFile, "r");
// if (!file) return NULL;

// char target[128];
// snprintf(target, sizeof(target), "$%s", SectionName);

// size_t capacity = 512;
// char* fullLang = malloc(capacity);
// if (!fullLang) { fclose(file); return NULL; }
// fullLang[0] = '\0';

// int found = 0;
// char buffer[1024];

// while (fgets(buffer, sizeof(buffer), file) != NULL) { // ← фикс бага 1
//     if (buffer[0] == '$') {
//         buffer[strcspn(buffer, "\r\n")] = 0;y
//         if (found) break;
//         if (strcmp(buffer, target) == 0) found = 1;
//         continue;
//     }
//     if (found) {
//         if (buffer[0] == '#') continue;
//         size_t addedLen  = strlen(buffer);
//         size_t currentLen = strlen(fullLang);
//         if (currentLen + addedLen + 1 > capacity) { // ← фикс бага 2
//             capacity = currentLen + addedLen + 512;
//             char* temp = realloc(fullLang, capacity);
//             if (!temp) { free(fullLang); fclose(file); return NULL; }
//             fullLang = temp;
//         }
//         strcat(fullLang, buffer);
//     }
// }

// fclose(file);
// if (strlen(fullLang) == 0) { free(fullLang); return NULL; }
// return fullLang;






char* getSection(char* SectionName, const char* langFile, signed char mode) {
    size_t SectionNameLength = strlen(SectionName);
    FILE* fl = fopen(langFile, "r");
    if (fl == NULL){
        error("Failed to open file (getSection)");
        write(1, C_ERROR, sizeof(C_ERROR));
        perror("[Error]:");
        write(1, C_RESET, sizeof(C_RESET));
        return NULL;
    }

    fseek(fl, 0, SEEK_END);
    long Length = ftell(fl);
    fseek(fl, 0, SEEK_SET);

    if (Length < 0){
        error("Failed to determine file size (getSection)");
        fclose(fl);
        return NULL;
    }

    char *buf = malloc((size_t)Length + 1);
    if (buf == NULL){
        error("Memory allocation error! (getSection: buf)");
        fclose(fl);
        return NULL;
    }

    size_t bytesRead = fread(buf, 1, (size_t)Length, fl);
    if (bytesRead != (size_t)Length){
        error("Failed to read file to buffer! (getSection)");
        free(buf);
        fclose(fl);
        return NULL;
    }

    buf[Length] = '\0';
    size_t i = 0;
    long ValuefirstCharIndex = -1;
    short found = 0;

    while (buf[i] != '\0'){
        if (buf[i] == '$' && buf[i + 1] != '\n'){
            found = 1;

            for (size_t a = 0; a < SectionNameLength; a++){
                if (i + 1 + a >= (size_t)Length || buf[i + 1 + a] != SectionName[a]){
                    found = 0;
                    break;
                }
            }

            if (found == 1 &&
                (i + 1 + SectionNameLength >= (size_t)Length ||
                 buf[i + 1 + SectionNameLength] != '\n')){
                found = 0;
            }

            if (found == 1){
                while (buf[i] != '\n' && buf[i] != '\0') i++;
                if (buf[i] == '\n') i++;
                ValuefirstCharIndex = (long)i;
                break;
            }
        }
        i++;
    }

    if (ValuefirstCharIndex == -1){
        error("Section not found (getSection)");
        free(buf);
        fclose(fl);
        return NULL;
    }

    size_t ValueLength = 0;
    if (mode == 0){
        i = (size_t)ValuefirstCharIndex;
        while (buf[i] != '$' && buf[i] != '\0'){
            ValueLength++;
            i++;
        }
    }
    else{
        i = (size_t)ValuefirstCharIndex;
        while (buf[i] != '\n' && buf[i] != '\0'){
            ValueLength++;
            i++;
        }
    }

    char *Value = malloc(ValueLength + 1);
    if (Value == NULL){
        error("Memory allocation error! (getSection: Value)");
        free(buf);
        fclose(fl);
        return NULL;
    }

    i = (size_t)ValuefirstCharIndex;
    for (size_t c = 0; c < ValueLength; c++){
        Value[c] = buf[i];
        i++;
    }
    Value[ValueLength] = '\0';

    free(buf);
    fclose(fl);

    return Value;
}






int WriteToSection(const char* SectionName, const char* Value, const char* path){

    size_t SectionNameLength = strlen(SectionName);
    size_t ValueLength = strlen(Value);
    FILE* fl = fopen(path,"r+");
    
    if(fl == NULL){
        error("Failed to open file (WriteToSection)");
        return -1;
    }
    
    fseek(fl,0,SEEK_END);
    long Length = ftell(fl);
    fseek(fl,0,SEEK_SET);
    char *buf = malloc(Length + ValueLength + 1); //Buf overflow solve
    if (buf == NULL){
        error("Memory allocation error! (WriteToSection: buf)");
        fclose(fl);
        return -2;
    }
    
    size_t bytesRead = fread(buf,1,Length,fl);
    
    if (bytesRead != (size_t)Length){ //If read bytes not = file length print error and terminate function
        error("Failed to read file to buffer!");
        free(buf);
        buf = NULL;
        fclose(fl);
        return -3;
    }


    buf[Length] = '\0';
    
    size_t i = 0;
    long ValuefirstCharIndex = -1;
    short found = 0;
    while (buf[i] != '\0'){
        if (buf[i] == '$' && buf[i + 1] != '\n'){
            found = 1;
            for (size_t a = 0; a < SectionNameLength; a++){
                if (buf[i + 1 + a] != SectionName[a]){
                    found = 0;
                    break;
                }
            }
            if (found == 1 && buf[i + 1 + SectionNameLength] != '\n'){
                found = 0;
            }
            if (found == 1){
                while (buf[i] != '\n' && buf[i] != '\0') i++;
                if (buf[i] == '\n') i++;
                ValuefirstCharIndex = (long)i;
                break;
            }
        }
        i++;
    }
    if (ValuefirstCharIndex == -1){
        error("Section not found (WriteToSection)");
        free(buf);
        fclose(fl);
        return -4;
    }


    size_t OldDataLength = 0;
    i = ValuefirstCharIndex;
    while (buf[i] != '\n' && buf[i] != '\0'){
        OldDataLength++;
        i++;
    }
    
    size_t tailLen = Length - (ValuefirstCharIndex + OldDataLength);
    memmove(buf + ValuefirstCharIndex + ValueLength, buf + ValuefirstCharIndex + OldDataLength,  tailLen + 1);

    i = ValuefirstCharIndex;
    for (size_t b = 0; b < ValueLength; b++){
        buf[i] = Value[b];
        i++;
    }
        
    size_t newLength = Length - OldDataLength + ValueLength;
    fseek(fl, 0, SEEK_SET);
    if (ValueLength < OldDataLength){
        ftruncate(fileno(fl), (size_t)newLength);
    }
    if (fwrite(buf, 1, newLength, fl) != newLength){
        error("Failed to write data to file! (WriteToSection)");
        free(buf);
        fclose(fl);
        return -5;
    }


    free(buf);
    fclose(fl);


    return 0;
}

// char *getlangline(int n, const char* langfile) { //Deleted for custom localizations dont be broked
//     FILE *f = fopen(langfile, "r");
//     if (!f) return NULL;

//     char buf[256];
//     int current = 0;

//     while (fgets(buf, sizeof(buf), f)) {
//         if (current == n) {
//             fclose(f);
//             char *nl = strchr(buf, '\n');
//             if (nl) *nl = '\0';
//             return strdup(buf);
//         }
//         if (current > n){
//             break;
//         }
//         current++;
//     }

//     fclose(f);
//     return NULL;
// }