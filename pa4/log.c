#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "common.h"
#include "log.h"

int log_open(int fds[]) {
	int fd_ev = open(events_log, O_WRONLY | O_APPEND | O_CREAT, 0644);
	int fd_pp = open(pipes_log, O_WRONLY | O_APPEND | O_CREAT, 0644);
	fds[0] = fd_ev;
	fds[1] = fd_pp;
	return 0;
}

int log_close(int fds[]) {
	int close_ev = close(fds[0]);
	int close_pp = close(fds[1]);
	return !close_ev && !close_pp;
}

void static log_message(int fd, const char* fmt, int num, ...) {
	va_list valist;
	va_start(valist, num);

	char buff[MAX_MES_LEN];
	int len = vsnprintf(buff, MAX_MES_LEN, fmt, valist);
	printf("%s", buff);
	write(fd, buff, len);
	va_end(valist);
}

void static log_message_np(int fd, const char* fmt, int num, ...) {
	va_list valist;
	va_start(valist, num);

	char buff[MAX_MES_LEN];
	int len = vsnprintf(buff, MAX_MES_LEN, fmt, valist);
	write(fd, buff, len);
	va_end(valist);
}

void log_init_pipe(int fd, int fds[2], local_id id) {
	char buff[MAX_MES_LEN];
	int len = snprintf(buff, MAX_MES_LEN, "Opened pipe [r: %d; w: %d] on process %d\n", fds[0], fds[1], id);
	write(fd, buff, len);
}

void log_close_pipe(int fd, int p_fd, local_id id) {
	char buff[MAX_MES_LEN];
	int len = snprintf(buff, MAX_MES_LEN, "Closed pipe %d on process %d\n", p_fd, id);
	write(fd, buff, len);
}

void log_loop_operation(int fd, local_id local, int i, int count) {
	log_message_np(fd, log_loop_operation_fmt, 3, local, i, count);
}

void log_started(int fd, timestamp_t time, local_id local, pid_t current_pid, pid_t parrent_pid, balance_t balance) {
	log_message(fd, log_started_fmt, 5, time, local, current_pid, parrent_pid, balance);
}

void log_received_all_started(int fd, timestamp_t time, local_id id) {
	log_message(fd, log_received_all_started_fmt, 2, time, id);
}

void log_done(int fd, timestamp_t time, local_id id, balance_t balance) {
	log_message(fd, log_done_fmt, 3, time, id, balance);
}

void log_received_all_done(int fd, timestamp_t time, local_id id) {
	log_message(fd, log_received_all_done_fmt, 2, time, id);
}

void log_transfer_in(int fd, timestamp_t time, local_id src, balance_t amount, local_id dst) {
	log_message(fd, log_transfer_in_fmt, 4, time, src, amount, dst);
}

void log_transfer_out(int fd, timestamp_t time, local_id src, balance_t amount, local_id dst) {
	log_message(fd, log_transfer_out_fmt, 4, time, src, amount, dst);
}


void log_error(char* message) {
	printf("ERROR: %s\n", message);
}
