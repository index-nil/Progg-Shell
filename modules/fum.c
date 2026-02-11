// #include "mongoose.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #ifdef _WIN32
//   #define WIN32_LEAN_AND_MEAN
//   #include <winsock2.h>
// #endif

#ifdef _WIN32

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <dirent.h>
    #include <sys/stat.h>
#endif
#include "UIlib.h"
#include "fum.h"
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
        
         
        if (buffer[i] != '\r'|| buffer[i] != '\t') {
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


// static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
//     struct download_status *status = (struct download_status *) c->fn_data;

//     if (ev == MG_EV_READ) {
//         // Данные пришли в буфер c->recv
//         // Нам нужно распарсить заголовки только один раз, чтобы узнать длину
//         if (status->content_len == 0) {
//             struct mg_http_message hm;
//             int res = mg_http_parse((char *) c->recv.buf, c->recv.len, &hm);
//             if (res > 0) { // Заголовки получены
//                 struct mg_str *cl = mg_http_get_header(&hm, "Content-Length");
//                 if (cl) status->content_len = (size_t) strtoull(cl->buf, NULL, 10);
                
//                 // Сдвигаем указатель записи в файл, чтобы не записать заголовки в сам файл
//                 // Но проще всего в Mongoose 7 дождаться конца заголовков:
//                 // Данные тела начинаются после hm.body.ptr
//             }
//         }

//         // Если мы уже в процессе скачивания тела
//         // В упрощенном виде для маленьких файлов пишем всё в MG_EV_HTTP_MSG
//         // Но для БОЛЬШИХ файлов пишем здесь и очищаем буфер:
//     } 
//     else if (ev == MG_EV_HTTP_MSG) {
//         struct mg_http_message *hm = (struct mg_http_message *) ev_data;
        
//         // Записываем всё тело сразу
//         if (status->fp != NULL && hm->body.len > 0) {
//             fwrite(hm->body.buf, 1, hm->body.len, status->fp);
//         }

//         create_pb("[Download]: ", 1, 100, 20, "∎", "-", "|", 1, 92);
//         printf("\n[Download]: Success! File saved.\n");
//         status->is_done = true;
//     } 
//     else if (ev == MG_EV_ERROR) {
//         printf("\n[Download]: Error: %s\n", (char *) ev_data);
//         status->is_done = true;
//     }
// }
// void download(const char *url, const char *filename) {
//     mg_log_set(0); // Жестко отключаем все уровни логирования
//     struct mg_mgr mgr;
//     struct download_status status = {0};
    
//     status.fp = fopen(filename, "wb");
//     if (!status.fp) return;

//     mg_mgr_init(&mgr);
    
//     // В Mongoose 7.x mg_http_connect принимает URL строкой
//     struct mg_connection *c = mg_http_connect(&mgr, url, ev_handler, &status);
    
//     if (c == NULL) {
//         printf("[Download]: Failed to connect\n");
//         fclose(status.fp);
//         mg_mgr_free(&mgr);
//         return;
//     }

//     if (strncmp(url, "https", 5) == 0) {
//         mg_tls_init(c, NULL);
//     }

//     // ТАК КАК МЫ НЕ МОЖЕМ ИСПОЛЬЗОВАТЬ mg_url (incomplete type),
//     // МЫ ИСПОЛЬЗУЕМ ХЕЛПЕРЫ САМОЙ БИБЛИОТЕКИ ДЛЯ ОТПРАВКИ ЗАПРОСА
    
//     // В простейшем случае для большинства серверов сработает такой запрос:
//     mg_printf(c, "GET %s HTTP/1.0\r\n"
//                  "Host: %s\r\n"
//                  "User-Agent: Mongoose\r\n"
//                  "Connection: close\r\n\r\n", 
//                  url, "localhost"); 
//     // Примечание: Современные серверы поймут полный URL в строке GET

//     while (!status.is_done) {
//         mg_mgr_poll(&mgr, 100);
//     }

//     if (status.fp) fclose(status.fp);
//     mg_mgr_free(&mgr);
// }

// void get_filename_from_url(const char *url, char *buffer, size_t buffer_size) {
//     const char *last_slash = strrchr(url, '/');
//     const char *start = last_slash ? last_slash + 1 : url;
    
//     // Ищем конец имени (начало параметров ?) или конец строки
//     const char *query_params = strchr(start, '?');
//     size_t length;

//     if (query_params) {
//         length = query_params - start;
//     } else {
//         length = strlen(start);
//     }

//     if (length >= buffer_size) {
//         length = buffer_size - 1;
//     }

//     // Если имя пустое (например, ссылка заканчивается на /), ставим дефолтное
//     if (length == 0) {
//         strncpy(buffer, "downloaded_file.dat", buffer_size);
//     } else {
//         strncpy(buffer, start, length);
//         buffer[length] = '\0';
//     }
// }






// Определяем, какую библиотеку подключать
#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif

int is_directory(const char *path) {
#ifdef _WIN32
    DWORD dwAttrib = GetFileAttributesA(path); // 'A' для работы с char* (ANSI)

    return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
           (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        return 0; // Ошибка (файл не найден или нет доступа)
    }
    return S_ISDIR(statbuf.st_mode);
#endif
}