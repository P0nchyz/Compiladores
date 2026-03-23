#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

void *proj_memcpy(void *dest, const void *src, size_t count);

void *proj_memset(void *dest, unsigned char ch, size_t count);

#endif
