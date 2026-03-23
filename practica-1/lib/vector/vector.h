#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <stdbool.h>

typedef ptrdiff_t ssize_t;

typedef struct {
	void *data;
	size_t size;
	size_t capacity;
	size_t elem_size;
} Vector;

typedef bool (*VectorPredicate)(const void *a, const void *b);

void Vector_Init(Vector *v, size_t elem_size);

size_t Vector_Push(Vector *v, const void *data);

int Vector_Pop(Vector *v, void *out);

void *Vector_Get(const Vector *v, size_t index);

void Vector_Get_Copy(const Vector *v, size_t index, void *out);

void Vector_Set(const Vector *v, size_t index, const void *data);

bool Vector_IsEmpty(const Vector *v);

size_t Vector_Size(const Vector *v);

void Vector_Free(Vector *v);

ssize_t Vector_Find(const Vector *v, const void *key, VectorPredicate pred);

#endif
