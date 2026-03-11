#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>

typedef struct {
	void *data;
	unsigned int size;
	unsigned int capacity;
	unsigned int elem_size;
} Vector;

typedef bool (*VectorPredicate)(const void *a, const void *b);

void Vector_Init(Vector *v, unsigned int elem_size);

unsigned int Vector_Push(Vector *v, const void *data);

int Vector_Pop(Vector *v, void *out);

void *Vector_Get(Vector *v, unsigned int index);

void Vector_Get_Copy(Vector *v, unsigned int index, void *out);

void Vector_Set(Vector *v, unsigned int index, const void *data);

bool Vector_IsEmpty(Vector *v);

int Vector_Size(Vector *v);

void Vector_Free(Vector *v);

ssize_t Vector_Find(Vector *v, const void *key, VectorPredicate pred);
#endif
