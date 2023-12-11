#include "banking.h"
#include "lamport_clock.h"

enum {
	LAMPORT_CLOCK_INCREMENT = 1
};

timestamp_t cur_time = 0;

timestamp_t get_lamport_time() {
	return cur_time;
}

void lamport_rule1() {
	cur_time += LAMPORT_CLOCK_INCREMENT;
}

void lamport_rule2(timestamp_t message_time) {
	cur_time = (message_time > cur_time) ? message_time : cur_time;
}
