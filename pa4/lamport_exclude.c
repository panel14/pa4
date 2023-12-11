#include <stdio.h>
#include "pa2345.h"
#include "process.h"

int request_cs(const void* self) {
	process_t* process = (process_t*)self;

	Message request;
	create_message(CS_REQUEST, &request, 0);
	send_multicast(process, &request);

	queue_elem new_elem = { .id = process->id, .time = request.s_header.s_local_time };
	insert(new_elem, &(process->lamport_queue));

	return 0;
}

int release_cs(const void* self) {
	process_t* process = (process_t*)self;

	pop(&(process->lamport_queue));

	Message release;
	create_message(CS_RELEASE, &release, 0);
	send_multicast(process, &release);

	return 0;
}
