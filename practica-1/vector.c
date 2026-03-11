#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "vector.h"

void Vector_Init(Vector *v, unsigned int elem_size)
{
	v->size = 0;
	v->capacity = 256;
	v->elem_size = elem_size;
	v->data = malloc(v->capacity * elem_size);

	if (!v->data)
	{
		fprintf(stderr, "Vector allocation failed\n");
		exit(1);
	}
}

unsigned int Vector_Push(Vector *v, const void *data)
{
	if (v->size == v->capacity)
	{
		v->capacity *= 2;
		v->data = realloc(v->data, v->capacity * v->elem_size);
	}

	if (!v->data)
	{
		fprintf(stderr, "Vector allocation failed\n");
		exit(1);
	}

	unsigned int index = v->size;
	proj_memcpy((char *)v->data + v->size++ * v->elem_size, data, v->elem_size);

	return index;
}

int Vector_Pop(Vector *v, void *out)
{
	if (v->size == 0)
	{
		return 0;
	}

	v->size--;

	void *src = (char *)v->data + v->size * v->elem_size;

	if (out)
		proj_memcpy(out, src, v->elem_size);

	if (v->size > 256 && v->size == v->capacity / 4)
	{
		v->capacity /= 2;
		v->data = realloc(v->data, v->capacity * v->elem_size);

		if (!v->data)
		{
			fprintf(stderr, "Vector allocation failed\n");
			exit(1);
		}
	}

	return 1;
}

void *Vector_Get(Vector *v, unsigned int index)
{
	if (index >= v->size)
	{
		fprintf(stderr, "Index out of bounds\n");
		exit(1);
	}
	return (char *)v->data + index * v->elem_size;
}

void Vector_Get_Copy(Vector *v, unsigned int index, void *out)
{
	if (index >= v->size)
	{
		fprintf(stderr, "Index out of bounds\n");
		exit(1);
	}

	void *src = (char *)v->data + index * v->elem_size;

	if (out)
		proj_memcpy(out, src, v->elem_size);
}

void Vector_Set(Vector *v, unsigned int index, const void *data)
{
	if (index >= v->size)
	{
		fprintf(stderr, "Index out of bounds\n");
		exit(1);
	}
	proj_memcpy((char *)v->data + index * v->elem_size, data, v->elem_size);
}

bool Vector_IsEmpty(Vector *v)
{
	return !v->size;
}

int Vector_Size(Vector *v)
{
	return v->size;
}

void Vector_Free(Vector *v)
{
	free(v->data);
}

ssize_t Vector_Find(Vector *v, const void *key, VectorPredicate pred)
{
	for (size_t i = 0; i < v->size; i++)
	{
		void *elem = (char *)v->data + i * v->elem_size;

		if (pred(elem, key))
			return i;
	}

	return -1;
}
