#include <stdlib.h>

#include "memory.h"
#include "vector.h"

void Vector_Init(Vector *v, unsigned int elem_size)
{
	v->size = 0;
	v->capacity = 256;
	v->elem_size = elem_size;
	v->data = malloc(v->capacity * elem_size);
}


void Vector_Push(Vector *v, const void *data)
{
	if (v->size == v->capacity) {
		v->capacity *= 2;
		v->data = realloc(v->data, v->capacity * v->elem_size);
	}

	proj_memcpy((char *)v->data + v->size * v->elem_size, data, v->elem_size);
}

void *Vector_Get(Vector *v, unsigned int index)
{
	return (char *)v->data + index * v->elem_size;
}

void Vector_Set(Vector *v, unsigned int index, const void *data)
{
	proj_memcpy((char *)v->data + index * v->elem_size, data, v->elem_size);
}

void Vector_Free(Vector *v)
{
	free(v->data);
}
