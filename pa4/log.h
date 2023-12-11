#ifndef __IFMO_DISTRIBUTED_CLASS_LOG__H
#define __IFMO_DISTRIBUTED_CLASS_LOG__H

#include "banking.h"
#include "ipc.h"
#include "pa2345.h"

enum {
	MAX_MES_LEN = 85
};

int log_open(int fds[]);

int log_close(int fds[]);

void log_init_pipe(int fd, int fds[2], local_id id);

void log_close_pipe(int fd, int p_fd, local_id id);

void log_loop_operation(int fd, local_id local, int i, int count);

void log_started(int fd, timestamp_t time, local_id local, pid_t current_pid, pid_t parrent_pid, balance_t balance);

void log_received_all_started(int fd, timestamp_t time, local_id id);

void log_done(int fd, timestamp_t time, local_id id, balance_t balance);

void log_received_all_done(int fd, timestamp_t time, local_id id);

void log_transfer_in(int fd, timestamp_t time, local_id src, balance_t amount, local_id dst);

void log_transfer_out(int fd, timestamp_t time, local_id src, balance_t amount, local_id dst);

void log_error(char* message);

#endif // __IFMO_DISTRIBUTED_CLASS_LOG__H
