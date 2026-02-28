// Dynamic Tools
#ifndef DYNAMIC_H
#define DYNAMIC_H

#include "types.h"


void init_buffer(DynamicBuffer *db, size_t initial_capacity);
void append_to_buffer(DynamicBuffer *db, const char *new_data, size_t data_len);
void free_buffer(DynamicBuffer *db);
#endif
