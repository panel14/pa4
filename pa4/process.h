#ifndef __IFMO_DISTRIBUTED_CLASS_PROCESS__H
#define __IFMO_DISTRIBUTED_CLASS_PROCESS__H

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "log.h"
#include "queue.h"

typedef struct {
	int r_fd;
	int w_fd;
} pipe_t;

typedef struct {
	pid_t pid;
	local_id id;
	size_t process_count;
	pipe_t** pipes_fd;
	queue lamport_queue;
	queue_elem last_received;
	int logs[2];
} process_t;

int close_all_pipes(process_t* process);

int free_all_pipes(process_t* process);

int prepare_proc(process_t* process, int count);

int create_message(MessageType type, Message* msg, int num, ...);

void wait_all_type(int fd, process_t* process, MessageType type, int count);

#endif // __IFMO_DISTRIBUTED_CLASS_PA1__H
