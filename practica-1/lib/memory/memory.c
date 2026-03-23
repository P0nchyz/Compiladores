#include "memory.h"

void *proj_memcpy(void *dest, const void *src, size_t count)
{
	unsigned char *d = dest;
	const unsigned char *s = (const unsigned char *)src;
	for (unsigned int i = 0; i < count; i++) {
		d[i]  = s[i];
	}

	return d;
}

void *proj_memset(void *dest, unsigned char ch, size_t count)
{
	unsigned char *d = dest;

	for (unsigned int i = 0; i < count; i++) {
		d[i] = ch;
	}

	return  d;
}
