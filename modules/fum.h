#ifndef FILE_UTILS_H
#define FILE_UTILS_H

//File Utils Module header
char* read_file_to_buffer(const char* filename, long* out_length);


int count_words(const char* buffer, long length);


int count_symbols(const char* buffer, long length, bool hard_read);


#endif