#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "memory/memory.h"
#include "vector.h"

static void Vector_Resize(Vector *v, size_t new_size)
{
	v->capacity = new_size;
	v->data = realloc(v->data, new_size * v->elem_size);

	if (NULL == v->data) {
		fprintf(stderr, "Vector allocation failed\n");
		exit(1);
	}
}

void Vector_Init(Vector *v, size_t elem_size)
{
	v->size = 0;
	v->capacity = 256;
	v->elem_size = elem_size;
	v->data = malloc(v->capacity * elem_size);

	if (!v->data) {
		fprintf(stderr, "Vector allocation failed\n");
		exit(1);
	}
}

size_t Vector_Push(Vector *v, const void *data)
{
	if (v->size == v->capacity)
		Vector_Resize(v, v->capacity * 2);

	size_t index = v->size;
	proj_memcpy((char *)v->data + index * v->elem_size, data, v->elem_size);
	v->size++;

	return index;
}

int Vector_Pop(Vector *v, void *out)
{
	if (v->size == 0)
		return 0;

	v->size--;

	void *src = (char *)v->data + v->size * v->elem_size;

	if (out)
		proj_memcpy(out, src, v->elem_size);

	if (v->size > 256 && v->size == v->capacity / 4)
		Vector_Resize(v, v->capacity / 2);

	return 1;
}

void *Vector_Get(const Vector *v, size_t index)
{
	if (index >= v->size) {
		fprintf(stderr, "Index out of bounds\n");
		exit(1);
	}
	return (char *)v->data + index * v->elem_size;
}

void Vector_Get_Copy(const Vector *v, size_t index, void *out)
{
	if (index >= v->size) {
		fprintf(stderr, "Index out of bounds\n");
		exit(1);
	}

	void *src = (char *)v->data + index * v->elem_size;

	if (out)
		proj_memcpy(out, src, v->elem_size);
}

void Vector_Set(const Vector *v, size_t index, const void *data)
{
	if (index >= v->size) {
		fprintf(stderr, "Index out of bounds\n");
		exit(1);
	}
	proj_memcpy((char *)v->data + index * v->elem_size, data, v->elem_size);
}

bool Vector_IsEmpty(const Vector *v)
{
	return !v->size;
}

size_t Vector_Size(const Vector *v)
{
	return v->size;
}

void Vector_Free(Vector *v)
{
	free(v->data);
}

ssize_t Vector_Find(const Vector *v, const void *key, VectorPredicate pred)
{
	for (size_t i = 0; i < v->size; i++) {
		void *elem = (char *)v->data + i * v->elem_size;

		if (pred(elem, key))
			return (ssize_t)i;
	}

	return -1;
}
