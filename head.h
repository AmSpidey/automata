#ifndef ZADANIE3PW_HEAD_H
#define ZADANIE3PW_HEAD_H

#include <sys/prctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include<fcntl.h>           /* For O_* constants */
#include<sys/stat.h>        /* For mode constants */
#include <signal.h>
#define DEBUG false
#define debug_printf(fmt,...) do {if (DEBUG) printf(fmt, ##__VA_ARGS__); fflush(stdout);} while (0)
#define DEBUG_PRINTF_ERR(fmt,...) do {if (DEBUG) fprintf(stderr, fmt, ##__VA_ARGS__); fflush(stderr);} while (0)
#define alpha_num ('z'-'a')
#define max_state_count 100
#define MAXLEN 1000
#define link_rec_v "/rev_v"
#define PRINT_ERR(f,m, ...) do {if (f) system_err((m), ##__VA_ARGS__);} while (0)

typedef struct automaton {
    int beginning;
    int universals;
    int states_count;
    int alpha_size;
    int acc_size;
    int autom[max_state_count][alpha_num][max_state_count];
    bool accepted[max_state_count];
    int line_counter;
} Automaton;

void system_err (char* fmt, ...);
void system_err_noclean(char* fmt, ...);

bool is_a_num(char x);
void buffer_cleaner(char* buff, int buff_size);
void link_v_t_pid(char* link_name, int pid);

void open_m_que(char *q_name, mqd_t *desc);

void create_m_que(char *q_name, mqd_t *desc);

void write_message(mqd_t desc, char* message, int message_len, int prio);
int buffer_size (mqd_t desc, struct mq_attr* mq_a);

void read_message (mqd_t desc, char* buff, int buff_size);

#endif //ZADANIE3PW_HEAD_H
