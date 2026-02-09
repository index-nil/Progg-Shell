#ifndef FILE_UTILS_H
#define FILE_UTILS_H

//File Utils Module header

struct download_status {
    FILE *fp;
    size_t content_len;
    size_t received_len;
    bool is_done;
};
typedef struct {
    char** names;
    int count;
} DirList;

char* read_file_to_buffer(const char* filename, long* out_length);

int count_words(const char* buffer, long length);

int count_symbols(const char* buffer, long length);


int directory_exists(signed char *path);

int count_directories(const char *path);

char* CombinePath(const char *folder, const char *file);

DirList get_directories(const char* path);

void download(const char *url, const char *filename);

void get_filename_from_url(const char *url, char *buffer, size_t buffer_size);

#endif