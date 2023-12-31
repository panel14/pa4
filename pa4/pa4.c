
#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include <time.h>
#include "lamport_clock.h"
#include "process.h"
#include "pa2345.h"

void transfer(void* parent_data, local_id src, local_id dst,
    balance_t amount)
{
    TransferOrder order = { .s_src = src, .s_dst = dst, .s_amount = amount };
    Message transfer;

    create_message(TRANSFER, &transfer, 1, order);
    send(parent_data, src, &transfer);
}

void static wait_all_exit() {
    while (wait(NULL) > 0);
}

int main(int argc, char* argv[])
{
    //clock_t begin = clock();

    process_t main;
    int mutex = 0;

    int proc_count = atoi(argv[2]);

    if (proc_count > MAX_PROCESS_ID) {
        printf("Max number of process: %d. Given: %d\n", MAX_PROCESS_ID, proc_count);
        return -1;
    }

    if (argc > 3 && strcmp("--mutexl", argv[3]) == 0) {
        mutex = 1;
    }

    int fds[2];
    log_open(fds);
    main.logs[0] = fds[0];
    main.logs[1] = fds[1];

    //Queue Test
    //queue q1, q2;
    //init(&q1); //init(&q2);
    //
    //queue_elem e1 = { .id = 2, .time = 68 };
    //queue_elem e2 = { .id = 3, .time = 71 };
    ////queue_elem e3 = { .id = 2, .time = 1 };
    ////queue_elem e4 = { .id = 3, .time = 1 };
    //
    //insert(e1, &q1); //insert(e4, &q2);
    //insert(e2, &q1); //insert(e3, &q2);
    ////insert(e3, &q1); //insert(e2, &q2);
    ////insert(e4, &q1); //insert(e1, &q2);
    //
    //print_queue(&q1); //print_queue(&q2);
    //
    //queue_elem peeked;
    //pop(&q1);
    //peek(&q1, &peeked);
    //printf("<id: %d; time: %d>\n", peeked.id, peeked.time);
    //print_queue(&q1);

    prepare_proc(&main, proc_count);

    if (main.id == PARENT_ID) {
        log_started(main.logs[0], get_lamport_time(), main.id, main.pid, getppid(), 0);
        wait_all_type(main.logs[0], &main, STARTED, main.process_count);
        wait_all_type(main.logs[0], &main, DONE, main.process_count);
        wait_all_exit();
    }
    else {
        char buffer[MAX_MES_LEN];

        queue queue;
        init(&queue);
        main.lamport_queue = queue;

        Message start;
        create_message(STARTED, &start, 4, main.id, main.pid, getppid(), 0);
        send_multicast(&main, &start);
        log_started(main.logs[0], start.s_header.s_local_time, main.id, main.pid, getppid(), 0);

        wait_all_type(main.logs[0], &main, STARTED, main.process_count - 1);
        Message reply;
        Message done_msg;

        int count = main.id * 5;
        int cur_count = 0;
        int done = 0;
        int reply_count = 0;
        int is_doned = 0;

        queue_elem new_el = {0};
        timestamp_t end;

        create_message(CS_REPLY, &reply, 0);

        request_cs(&main);

        if (mutex) {
            while (done != main.process_count) {
                Message awaiting;
                int from = -1;

                while (from == -1) {
                    from = receive_any(&main, &awaiting);
                }
                //printf("%d: res after receive from: %d: time: %d\n", main.id, from, awaiting.s_header.s_local_time);
                switch (awaiting.s_header.s_type)
                {
                case CS_REQUEST:
                    printf("%d: request from: %d; time: %d\n", main.id, from, awaiting.s_header.s_local_time);

                    new_el.id = from; new_el.time = awaiting.s_header.s_local_time;
                    insert(new_el, &(main.lamport_queue));
                    send(&main, from, &reply);
                    break;

                case CS_RELEASE:
                    pop(&(main.lamport_queue));
                    break;

                case CS_REPLY:
                    reply_count++;
                    break;

                case DONE:
                    done++;
                    break;

                default:
                    break;
                }

                if (cur_count < count) {
                    queue_elem cur;
                    peek(&(main.lamport_queue), &cur);

                    if ((cur.id == main.id) && (reply_count == main.process_count - 1)) {

                        sprintf(buffer, log_loop_operation_fmt, main.id, cur_count + 1, count);
                        print(buffer);
                        log_loop_operation(main.logs[0], main.id, cur_count + 1, count);

                        release_cs(&main);
                        cur_count++;
                        reply_count = 0;

                        if (cur_count < count) {
                            request_cs(&main);
                        }
                    }
                }

                if (cur_count == count && !is_doned) {
                    end = get_lamport_time();
                    create_message(DONE, &done_msg, 3, end, main.id, 0);
                    send_multicast(&main, &done_msg);
                    log_done(main.logs[0], end, main.id, 0);

                    done++;
                    is_doned = 1;
                }
            }
        }
        else {

            for (int i = 0; i < count; i++) {
                sprintf(buffer, log_loop_operation_fmt, main.id, i, count);
                print(buffer);
            }

            end = get_lamport_time();
            create_message(DONE, &done_msg, 3, end, main.id, 0);
            send(&main, PARENT_ID, &done_msg);
            log_done(main.logs[0], end, main.id, 0);

            wait_all_type(main.logs[0], &main, DONE, main.process_count - 1);
        }
        exit(0);
    }

    log_close(fds);
    free_all_pipes(&main);

    //clock_t end = clock();
    //double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    //
    //printf("Execution time: %f (s)\n", time_spent);

    return 0;
}
