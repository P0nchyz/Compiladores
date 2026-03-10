#ifndef QUEUE_H
#define QUEUE_H

typedef struct Node {
	void *data;
	struct Node *next;
} Node;

typedef struct {
	Node *front;
	Node *rear;
	unsigned int element_size;
	unsigned int size;
} Queue;

void QueueInit(Queue *q, unsigned int element_size);
void QueueDestroy(Queue *q);

bool Queue_Enqueue(Queue *q, const void *value);
bool Queue_Dequeue(Queue *q, void *out_value);
bool Queue_Peek(const Queue *q, void *out_value);

bool Queue_IsEmpty(const Queue *q);
unsigned int Queue_Size(const Queue *q);

#endif
