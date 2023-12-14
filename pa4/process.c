#include <fcntl.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "lamport_clock.h"
#include "process.h"

void wait_all_type(int fd, process_t* process, MessageType type, int count) {
	int received = 0;
	int from = -1;
	while (received != count) {
		Message msg;
		from = -1;
		while (from == -1) {
			from = receive_any(process, &msg);
		}

		if (msg.s_header.s_type == type) {
			//printf("%d: from: %d", process->id, from);
			received++;
		}

		if (type == CS_REQUEST) {
			printf("%d: request in all type awaiting\n", process->id);
		}
	}

	switch (type)
	{
	case STARTED:
		log_received_all_started(fd, get_lamport_time(), process->id);
		break;

	case DONE:
		log_received_all_done(fd, get_lamport_time(), process->id);
		break;

	default:
		break;
	}
}

int close_pipe_fd(int log_fd, local_id id, int fd) {
	int ret;
	if ((ret = close(fd)) == -1) {
		log_error("Pipe closing failed");
	}
	log_close_pipe(log_fd, fd, id);
	return ret;
}

int create_pipe(int log_fd, local_id id, pipe_t* pipe_s) {
	int fd[2];

	if (pipe(fd) == -1) {
		log_error("Pipe opening failed");
		return -1;
	}
	fcntl(fd[0], F_SETFL, O_NONBLOCK);
	fcntl(fd[1], F_SETFL, O_NONBLOCK);

	pipe_s->r_fd = fd[0];
	pipe_s->w_fd = fd[1];
	log_init_pipe(log_fd, fd, id);
	return 0;
}

int create_all_pipes(process_t *process) {
	int count = process->process_count + 1;

	process->pipes_fd = (pipe_t**)malloc(count * sizeof(pipe_t*));
	for (int i = 0; i < count; i++) {
		process->pipes_fd[i] = (pipe_t*)malloc(count * sizeof(pipe_t));
	}
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count; j++) {
			if (i != j) {
				pipe_t p1;
				create_pipe(process->logs[1], process->id, &p1);
				process->pipes_fd[i][j] = p1;
			}
		}
	}

	return 0;
}

int close_unused_pipes(process_t* process) {
	int count = process->process_count + 1;

	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count; j++) {
			if (i != j) {
				if (i == process->id) {
					close_pipe_fd(process->logs[1], process->id, process->pipes_fd[i][j].r_fd);
				}
				else if (j == process->id) {
					close_pipe_fd(process->logs[1], process->id, process->pipes_fd[i][j].w_fd);
				}
				else {
					close_pipe_fd(process->logs[1], process->id, process->pipes_fd[i][j].r_fd);
					close_pipe_fd(process->logs[1], process->id, process->pipes_fd[i][j].w_fd);
				}
			}
		}
	}

	return 0;
}

int close_all_pipes(process_t* process) {
	int count = process->process_count + 1;

	for (int i = 0; i < count; i++) {
		for (int j = i; j < count; j++) {
			if (i != j) {
				close_pipe_fd(process->logs[1], process->id, process->pipes_fd[i][j].w_fd);
				close_pipe_fd(process->logs[1], process->id, process->pipes_fd[j][i].r_fd);
			}
		}
	}
	return 0;
}

int free_all_pipes(process_t* process) {
	int count = process->process_count + 1;
	
	for (int i = 0; i < count; i++) {
		free(process->pipes_fd[i]);
	}
	free(process->pipes_fd);

	return 0;
}

int init_parrent_proc(process_t* process, size_t count) {

	process->process_count = count;
	process->id = PARENT_ID;
	process->pid = getpid();

	return 0;
}

int create_message(MessageType type, Message* msg, int num, ...) {
	MessageHeader header = { 0 };
	TransferOrder order = { 0 };
	BalanceHistory history = { 0 };
	va_list valist;
	va_start(valist, num);

	header.s_type = type;
	header.s_magic = MESSAGE_MAGIC;
	
	lamport_rule1();
	header.s_local_time = get_lamport_time();

	switch (type)
	{
		case DONE:
			header.s_payload_len = vsnprintf(msg->s_payload, MAX_MES_LEN, log_done_fmt, valist);
			break;		
		case STARTED:
			header.s_payload_len = vsnprintf(msg->s_payload, MAX_MES_LEN, log_started_fmt, valist);
			break;
		case STOP:
			header.s_payload_len = 0;
			break;
		case TRANSFER:
			order = va_arg(valist, TransferOrder);
			header.s_payload_len = sizeof(TransferOrder);
			memcpy(msg->s_payload, &order, header.s_payload_len);
			break;
		case ACK:
			header.s_payload_len = 0;
		case CS_REQUEST:
			header.s_payload_len = 0;
			break;
		case CS_REPLY:
			header.s_payload_len = 0;
			break;
		case CS_RELEASE:
			header.s_payload_len = 0;
			break;
		case BALANCE_HISTORY:
			history = va_arg(valist, BalanceHistory);
			header.s_payload_len = sizeof(BalanceHistory);
			memcpy(msg->s_payload, &history, header.s_payload_len);
			break;
		default:
			break;
	}

	msg->s_header = header;
	va_end(valist);
	return 0;
}

int init_child_proc(process_t* process) {

	pid_t pid;

	int count = process->process_count;

	for (int i = 0; i < count; i++) {

		if (process->id == PARENT_ID) {
			pid = fork();

			if (pid == 0) {
				process->id = i + 1;
				process->pid = pid;
				return 0;
			}
		}
	}
	return 0;
}

void show_pipes(pipe_t** pipes, int count) {
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < count; j++) {
			printf("[r: %d; w:%d] ", pipes[i][j].r_fd, pipes[i][j].w_fd);
		}
		
		printf("\n");
	}
}

int prepare_proc(process_t* process, int count) {

	init_parrent_proc(process, count);
	create_all_pipes(process);
	//show_pipes(process->pipes_fd, process->process_count + 1);
	init_child_proc(process);
	close_unused_pipes(process);

	return 0;
}
