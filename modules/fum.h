#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stddef.h>
//File Utils Module header


size_t count_words(const char* buffer,  size_t length);

size_t count_symbols(const char* buffer, size_t length);

int is_directory(const char *path);

char* CombinePath(const char *folder, const char *file);

char* CombineStrings(const char* str_one, const char* str_two);

char* str_replace_all(const char* str, const char* find, const char* replace);


#endif