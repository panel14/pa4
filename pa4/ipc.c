
#include <stdio.h>
#include <unistd.h>
#include "ipc.h"
#include "process.h"
#include "log.h"
#include "lamport_clock.h"

int send(void* self, local_id dst, const Message* msg) {
	process_t* process = self;
	pipe_t pipe_s = process->pipes_fd[process->id][dst];

	write(pipe_s.w_fd, msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);
	return 0;
}

int send_multicast(void* self, const Message* msg) {
	process_t* process = self;
	for (local_id i = 0; i < process->process_count + 1; i++) {
		if (i != process->id) {
			send(process, i, msg);
		}
	}
	return 0;
}

int receive(void* self, local_id from, Message* msg) {
	process_t* process = self;
	pipe_t pipe_s = process->pipes_fd[from][process->id];

	int h_read = read(pipe_s.r_fd, msg, sizeof(MessageHeader));
	
	if (h_read == sizeof(MessageHeader) && msg->s_header.s_payload_len != 0 ) {
		read(pipe_s.r_fd, msg->s_payload, msg->s_header.s_payload_len);
	}

	if (h_read > 0) {
		lamport_rule2(msg->s_header.s_local_time);
		lamport_rule1();

		return from;
	}

	return -1;
}

int receive_any(void* self, Message* msg) {
	process_t* process = self;
	for (local_id i = 1; i < process->process_count + 1; i++) {
		if (i != process->id) {
			pipe_t pipe_s = process->pipes_fd[i][process->id];

			int h_read = read(pipe_s.r_fd, msg, sizeof(MessageHeader));

			if (h_read == sizeof(MessageHeader) /*&& msg->s_header.s_payload_len > 0*/) {
				h_read = read(pipe_s.r_fd, msg->s_payload, msg->s_header.s_payload_len);

				if (h_read == msg->s_header.s_payload_len) {
					lamport_rule2(msg->s_header.s_local_time);
					lamport_rule1();

					if (msg->s_header.s_type == CS_REQUEST)
						printf("%d: received from: %d; time: %d\n", process->id, i, msg->s_header.s_local_time);

					return i;
				}
			}
		}
	}
	return -1;
}
