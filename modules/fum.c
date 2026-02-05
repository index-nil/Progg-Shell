#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
#endif


//Progg Shell 2026.01.12 File module GPLv3
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
    signed char inWord = 1;

    for (long i = 0; i < length; i++) {
        // Проверяем: это "печатный" символ или разделитель?
        if (buffer[i] > 32) { // Все символы с кодом > 32 — это буквы, цифры и знаки
            if (!inWord) {
                inWord = 1;
                words++;
            }
        } else {
            inWord = 0;
        }
    }
    return words;
}

int directory_exists(signed char *path) {
    #ifdef _WIN32
        DWORD dwAttrib = GetFileAttributesA(path);
        return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
    #else
        struct stat sb;
        return (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode));
    #endif
}

int count_directories(const char *path) {
    int count = 0;
    #ifdef _WIN32
        WIN32_FIND_DATAA findData;
        char searchPath[MAX_PATH];

        // Создаем маску поиска: "folder/*"
        sprintf(searchPath, "%s/*", path);

        HANDLE hFind = FindFirstFileA(searchPath, &findData);

        if (hFind == INVALID_HANDLE_VALUE) {
            return 0; // Папка пуста или не существует
        }

        do {
            // Проверяем, что это директория
            if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                
                if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                    count++;
                }
            }
        } while (FindNextFileA(hFind, &findData));

        FindClose(hFind);
    #else
        struct dirent *entry;
        DIR *dp = opendir(path);

        if (dp == NULL) {
            return 0; // Не удалось открыть директорию
        }

        while ((entry = readdir(dp))) {
            // Проверяем, что это директория
            // DT_DIR поддерживается большинством файловых систем Linux
            if (entry->d_type == DT_DIR) {
                // Игнорируем "." и ".."
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    count++;
                }
            }
        }

        closedir(dp);
    
    #endif
    return count;
}



typedef struct {
    char** names;
    int count;
} DirList;


DirList get_directories(const char* path) {
    DirList result = {NULL, 0};
    
    // 1. Сначала считаем количество (используем твою логику)
    result.count = count_directories(path);
    if (result.count == 0) return result;

    // 2. Выделяем память под массив указателей
    result.names = (char**)malloc(result.count * sizeof(char*));

    int index = 0;

    #ifdef _WIN32
        WIN32_FIND_DATAA findData;
        char searchPath[MAX_PATH];
        sprintf(searchPath, "%s/*", path);
        HANDLE hFind = FindFirstFileA(searchPath, &findData);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0) {
                        // Копируем имя папки в массив
                        result.names[index] = _strdup(findData.cFileName); 
                        index++;
                    }
                }
            } while (FindNextFileA(hFind, &findData) && index < result.count);
            FindClose(hFind);
        }
    #else
        struct dirent *entry;
        DIR *dp = opendir(path);
        if (dp) {
            while ((entry = readdir(dp)) && index < result.count) {
                if (entry->d_type == DT_DIR) {
                    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                        result.names[index] = strdup(entry->d_name);
                        index++;
                    }
                }
            }
            closedir(dp);
        }
    #endif
    return result;
}

void free_dir_list(DirList list) {
    for (int i = 0; i < list.count; i++) {
        free(list.names[i]);
    }
    free(list.names);
}

char* CombinePath(const char *folder, const char *file) {
    if (!folder || !file) return NULL;


    size_t len = strlen(folder) + strlen(file) + 2; 
    
    char *newPath = (char*)malloc(len);
    if (!newPath){
        free(newPath);
        return NULL;
    } 


    snprintf(newPath, len, "%s/%s", folder, file);
    
    return newPath;
}
