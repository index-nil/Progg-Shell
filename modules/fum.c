#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


//Progg Shell 2026.01.12 File module GPL
//DONT FORGET ADD FREE()!!!

char* read_file_to_buffer(const char* filename, long* out_length) {
    FILE *file = fopen(filename, "rb"); // "rb" — чтение в бинарном режиме (надежнее)
    if (file == NULL) return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(length + 1);
    if (buffer) {
        fread(buffer, 1, length, file);
        buffer[length] = '\0';
        *out_length = length; // ВАЖНО: без этой строки filesize в main.c всегда будет 0!
    }

    fclose(file);
    return buffer;
}

int count_symbols(const char* buffer, long length){
    if (length <= 0 || !buffer) return 0;
    int words = (buffer[0] != ' ' && buffer[0] != '\n' && buffer[0] != '\r') ? 1 : 0;
    
    
    for (long i = 0; i < length - 1; i++)
    {
        
        if ((buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\r'|| buffer[i] == '\t') && 
            (buffer[i + 1] != ' ' && buffer[i + 1] != '\n' && buffer[i + 1] != '\r'|| buffer[i] != '\t')) {
            words++;
        }
    }
    
    return(words);
}


int count_words(const char* buffer, long length) {
    if (length <= 0 || !buffer) return 0;
    
    int words = 0;
    bool inWord = false;

    for (long i = 0; i < length; i++) {
        // Проверяем: это "печатный" символ или разделитель?
        if (buffer[i] > 32) { // Все символы с кодом > 32 — это буквы, цифры и знаки
            if (!inWord) {
                inWord = true;
                words++;
            }
        } else {
            inWord = false;
        }
    }
    return words;
}