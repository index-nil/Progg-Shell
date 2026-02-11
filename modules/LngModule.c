#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
#endif


//Progg Shell 2026.02.1 Language Module MIT License
//DONT FORGET ADD FREE()!!!



char* getSection(char* SectionName, char* langFile) {
    FILE* file = fopen(langFile, "r");
    if (!file) return NULL;

    char target[128];
    snprintf(target, sizeof(target), "$%s", SectionName);

    size_t capacity = 512;
    char* fullLang = malloc(capacity);
    if (!fullLang) { fclose(file); return NULL; }
    fullLang[0] = '\0';

    int found = 0;
    char* line = NULL;
    size_t lineCap = 0;

    // Читаем файл посимвольно, чтобы динамически подстраиваться под длину строки
    while (1) {
        char buffer[1024]; // Временный буфер для чтения куска строки
        if (fgets(buffer, sizeof(buffer), file) == NULL) break;

        // Если это заголовок секции
        if (buffer[0] == '$') {
            char check[1024];
            strcpy(check, buffer);
            check[strcspn(check, "\r\n")] = 0; // Чистим только для сравнения

            if (found) break; // Выходим, если началась новая секция
            if (strcmp(check, target) == 0) found = 1;
            continue;
        }

        if (found) {
            if (buffer[0] == '#') continue;

            size_t addedLen = strlen(buffer);
            size_t currentLen = strlen(fullLang);

            // Динамически расширяем итоговый буфер
            if (currentLen + addedLen + 1 > capacity) {
                capacity = currentLen + addedLen + 512;
                char* temp = realloc(fullLang, capacity);
                if (!temp) { free(fullLang); fclose(file); return NULL; }
                fullLang = temp;
            }
            strcat(fullLang, buffer); // Перенос \n берется прямо из файла!
        }
    }

    fclose(file);
    if (strlen(fullLang) == 0) { free(fullLang); return NULL; }
    return fullLang;
}