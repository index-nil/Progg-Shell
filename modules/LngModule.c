#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
#endif


//Progg Shell 2026.02.1 Language Module GPLv3
//DONT FORGET ADD FREE()!!!



char* getSection(char* SectionName, char* langFile) {
    FILE* file = fopen(langFile, "r");
    if (!file) {
        perror("Failed to open language file!");
        return NULL;
    }

    size_t currentCapacity = 256;
    char* fullLang = malloc(currentCapacity);
    if (!fullLang) { fclose(file); return NULL; }
    fullLang[0] = '\0'; 

    int Found = 0;
    char line[256];
    char target[128];
    snprintf(target, sizeof(target), "$%s", SectionName);

    while (fgets(line, sizeof(line), file)) {
        // Убираем только символы переноса для сравнения заголовка
        line[strcspn(line, "\r\n")] = 0;

        if (line[0] == '$') {
            if (Found == 1) break; // Секция закончилась
            if (strcmp(line, target) == 0) Found = 1; // Секция началась
            continue; 
        }

        // Если мы внутри нужной секции
        if (Found == 1) {
            if (line[0] == '#' || line[0] == '\0') continue;

            // Возвращаем перенос строки для корректного вывода
            strcat(line, "\n"); 

            size_t lineLen = strlen(line);
            size_t currentLen = strlen(fullLang);

            if (currentLen + lineLen + 1 > currentCapacity) {
                currentCapacity = currentLen + lineLen + 128; 
                char* temp = realloc(fullLang, currentCapacity);
                if (!temp) { free(fullLang); fclose(file); return NULL; }
                fullLang = temp;
            }
            strcat(fullLang, line);
        }
    }

    fclose(file);

    // Если ничего не нашли, освобождаем память и возвращаем NULL
    if (strlen(fullLang) == 0) {
        free(fullLang);
        return NULL;
    }

    return fullLang;
}