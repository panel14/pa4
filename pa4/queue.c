#include "queue.h"

void init(queue* queue) {
	queue->size = 0;
}

static void swap(queue_elem* a, queue_elem* b) {
	queue_elem temp = *b;
	*b = *a;
	*a = temp;
}

static int compare(queue_elem a, queue_elem b) {
	if (a.time > b.time) {
		return 1;
	}
	else if (a.time < b.time) {
		return -1;
	}
	else {
		if (a.id > b.id) {
			return 1;
		}
		else if (a.id < b.id) {
			return -1;
		}
		else return 0;
	}
}

static void sort_queue(queue* queue) {
	if (queue->size < 1) {
		return;
	}

	for (int i = 0; i < queue->size-1; i++) {
		int min_idx = i;

		for (int j = i + 1; j < queue->size; j++) {
			if (compare(queue->queue[j], queue->queue[min_idx]) == -1) {
				min_idx = j;
			}
		}
		swap(&queue->queue[min_idx], &queue->queue[i]);
	}
}

void insert(queue_elem elem, queue* queue) {
	queue->queue[queue->size] = elem;
	queue->size++;
	sort_queue(queue);
}

int peek(queue* queue, queue_elem* peeked) {
	if (queue->size > 0) {
		*peeked = queue->queue[0];
		return 0;
	}
	return -1;
}

int pop(queue* queue) {

	if (queue->size > 0) {
		
		for (int i = 1; i < queue->size; i++) {
			queue->queue[i - 1] = queue->queue[i];
		}
		queue->size--;
		return 0;
	}
	return 1;
}

void print_queue(queue* queue) {
	for (int i = 0; i < queue->size; i++) {
		printf("<id: %d; time: %d> ", queue->queue[i].id, queue->queue[i].time);
	}
	printf("\n");
}
