#ifndef __IFMO_DISTRIBUTED_CLASS_QUEUE__H
#define __IFMO_DISTRIBUTED_CLASS_QUEUE__H
#include <stdio.h>
#include "ipc.h"

enum {
	MAX_QUEUE_LEN = 16
};

typedef struct {
	local_id id;
	timestamp_t time;
} queue_elem;

typedef struct {
	int size;
	queue_elem queue[MAX_QUEUE_LEN];
} queue;

void init(queue* queue);

void insert(queue_elem elem, queue* queue);

int peek(queue* queue, queue_elem* peeked);

int pop(queue* queue);

void print_queue(queue* queue);

#endif // __IFMO_DISTRIBUTED_CLASS_QUEUE__H
