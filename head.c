//
// Created by reo on 06.01.18.
//

#include "head.h"
void cleaner_callback(int sig);
void cleaner_when_sigint();

void cleaner (bool err);
void buffer_cleaner(char* buff, int buff_size) {
    for (int i=0; i<buff_size; i++) {
        buff[i]=0;
    }
}

void system_err(char* fmt, ...) {

    va_list fmt_args;

    fprintf(stderr, "ERROR: ");

    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end (fmt_args);
    fprintf(stderr," (%d; %s)\n", errno, strerror(errno));
    cleaner(1);
    exit(-1);
}

void system_err_noclean(char* fmt, ...) {
    va_list fmt_args;

    fprintf(stderr, "ERROR: ");

    va_start(fmt_args, fmt);
    vfprintf(stderr, fmt, fmt_args);
    va_end (fmt_args);
    fprintf(stderr," (%d; %s)\n", errno, strerror(errno));
}

bool is_a_num(char x) {
    return ('0'<=x && x<='9');
}

void create_m_que(char *q_name, mqd_t *desc) {
    struct mq_attr attr;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 2048;
    *desc = mq_open(q_name, O_RDWR | O_CREAT, 0755, &attr);
    if((*desc) == (mqd_t) -1)
        system_err("Error in mq_open");
}

void link_v_t_pid(char* link_name, int pid) {
    sprintf(link_name, "/link_v_t_%d", pid);
}

void open_m_que(char *q_name, mqd_t *desc) {
    *desc = mq_open(q_name, O_RDWR);
    if((*desc) == (mqd_t) -1)
        system_err("Error in mq_open");
}

void report_yourself();

void write_message(mqd_t desc, char* message, int message_len, int prio) {
    int wrote = mq_send(desc, message, message_len, prio);
    if (wrote) {
        system_err("Error in write\n");
    } else {
        debug_printf("Writing: %s\n", message);
    }
}

int buffer_size (mqd_t desc, struct mq_attr* mq_a) {
    if (mq_getattr(desc, mq_a))
        system_err("Error in getattr");
    int buff_size = (*mq_a).mq_msgsize + 1;
    return buff_size;
}

void read_message (mqd_t desc, char* buff, int buff_size) {
    int ret = mq_receive(desc, buff, buff_size, NULL);
    if (ret < 0) {
        system_err("Error in rec: ");
    } else {
        debug_printf("Read message \"%s\"\n", buff);
    }
}