#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

typedef struct {
	void *data;
	unsigned int size;
	unsigned int capacity;
	unsigned int elem_size;
} Vector;

void Vector_Init(Vector *v, unsigned int elem_size);

void Vector_Push(Vector *v, const void *data);

void *Vector_Get(Vector *v, unsigned int index);

void Vector_Set(Vector *v, unsigned int index, const void *data);

void Vector_Free(Vector *v);

#endif
