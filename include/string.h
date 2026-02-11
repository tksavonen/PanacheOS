// string.h

#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void* memcpy(void*dst, const void *src, size_t n);
void* memset(void* dst, int value, size_t n);
int strcmp(char* a, char *b);
size_t strlen(const char* s);

#endif
