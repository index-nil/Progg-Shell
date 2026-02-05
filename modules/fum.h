#ifndef FILE_UTILS_H
#define FILE_UTILS_H

//File Utils Module header
typedef struct {
    char** names;
    int count;
} DirList;

char* read_file_to_buffer(const char* filename, long* out_length);

signed char count_words(const char* buffer, long length);

int count_symbols(const char* buffer, long length, signed char hard_read);


int directory_exists(signed char *path);

int count_directories(const char *path);

char* CombinePath(const char *folder, const char *file);

DirList get_directories(const char* path);

#endif