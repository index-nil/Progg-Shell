
#include "types.h"
#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>







void init_buffer(DynamicBuffer *db, size_t initial_capacity) {
    db->capacity = initial_capacity > 0 ? initial_capacity : 16;
    db->size = 0;
    db->data = (char *)malloc(db->capacity);
}
void append_to_buffer(DynamicBuffer *db, const char *new_data, size_t data_len) {
    while (db->size + data_len >= db->capacity) {
        db->capacity += 16; //Add 16 bytes 
        char *temp = (char *)realloc(db->data, db->capacity);
        if (!temp) {
            fprintf(stderr, "\033[38;2;209;0;0mError\033[0m\n");
            return;
        }
        db->data = temp;
    }
    
    memcpy(db->data + db->size, new_data, data_len);
    db->size += data_len;
    db->data[db->size] = '\0'; 
}
void free_buffer(DynamicBuffer *db) {
    free(db->data);
    db->data = NULL;
    db->size = db->capacity = 0;
}
