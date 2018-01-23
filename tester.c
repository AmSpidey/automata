#include "head.h"

bool is_a_child = false;
char link_v_t[20];
pid_t pid, child_pid;
mqd_t desc_v;
mqd_t rec_v;

int rcd = 0;
int acc = 0;

void cleaner(bool err) {
    debug_printf("JESTEM TESTER%d I SPRZATAM\n", pid);
    if (err) {
        if (is_a_child) {
            kill(pid, SIGINT);
        } else {
            kill(child_pid, SIGINT);
        }
    } else if (!is_a_child) {
        kill(child_pid, SIGUSR1);
    }
    if (is_a_child) {
        return;
    }
    if (mq_unlink(link_v_t) == -1) {
        system_err_noclean("Error in unlinking mq");
    }
    if (mq_close(rec_v) == -1) {
        system_err_noclean("Error in closin mq");
    }
    if (mq_close(desc_v) == -1) {
        system_err_noclean("Error in closing mq");
    }
    wait(0);
}

void end_vali() {

}

int word_for_vali(char* query) {
    sprintf(query, "%s %d", query, pid);
    return strlen(query);
}

void cleaner_callback(int sig) {
    debug_printf("Received signal %s, cleaning up.\n", strsignal(sig));
    if (is_a_child) {
        kill(child_pid, SIGINT);
    }
    cleaner(sig != SIGUSR1);
    exit(0);
}

void cleaner_when_sigint() {
    struct sigaction action; /*reprezentacja zachowania programu
 * podczas odebrania sygnalu*/
    action.sa_handler = cleaner_callback;

    sigset_t mask;
    if (sigemptyset(&mask)) {
        system_err("error in sigemptyset");
    }
    if (sigaddset(&mask, SIGINT)) {
        system_err("error in sigaddset");
    }
    if (sigaddset(&mask, SIGUSR1)) {
        system_err("error in sigaddset");
    }
    action.sa_mask = mask;
    action.sa_flags = 0;
    if (sigaction(SIGINT, &action, 0) == -1) {
        system_err("error in sigaction");
    }
    if (sigaction(SIGUSR1, &action, 0) == -1) {
        system_err("error in sigaction");
    }
}

void report_yourself() {
    printf("Rcd: %d\nSnt: %d\nAcc: %d\n", rcd, rcd, acc);
}


void test() {
    cleaner_when_sigint();
    pid = getpid();
    int oficjalnypid = pid;
    int to_receive = -666;
    struct mq_attr mq_v_r;
    sprintf(link_v_t, "/link_v_t_%d", pid);
    debug_printf("TESTER (pid: %d) STARTING\n", pid);
    create_m_que(link_v_t, &rec_v);
    debug_printf("TESTER (pid: %d) created que for receiving\n", pid);
    open_m_que(link_rec_v, &desc_v);
    debug_printf("TESTER (pid: %d) opened queue for sending\n", pid);

    int rec_v_size = buffer_size(rec_v, &mq_v_r);
    char receive_v[rec_v_size];

    switch (child_pid = fork()) {
    case -1:
        system_err("Error in fork\n");
        break;

    case 0:
        child_pid = getpid();
        printf("PID: %d\n", oficjalnypid);
        is_a_child = true;
        while (true) {
            char* current_query = NULL;
            debug_printf("KID: próbuję  czytać\n");
            current_query = NULL;
            size_t len = 0;
            getline(&current_query, &len, stdin);
            debug_printf("KID: przeczytałem \n");
            if (feof(stdin)) {
                debug_printf("TESTERkid (pid: %d) found feof\n", pid);
                free(current_query);
                break;
            }
            if (current_query == NULL) {
                free(current_query);
                break;
            } else if (current_query[0] == '!') {
                debug_printf("TESTERkid (pid: %d) received ! from stdin\n", pid);
                debug_printf("Tester%s pisze: %s\n", (is_a_child ? "kid" : ""), current_query);
                char exclam[25];
                for (int i = 0; i < 25; ++i) {
                    exclam[i] = '\0';
                }
                sprintf(exclam, "! %d", oficjalnypid);
                write_message(desc_v, exclam, 25, 1);
                end_vali();
                free(current_query);
                break;
            } else {
                debug_printf("TESTERkid (pid: %d) received word\n", pid);
                int mess_for_vali_l = word_for_vali(current_query);
                debug_printf("Tester%s pisze: %s\n", (is_a_child ? "kid" : ""), current_query);
                write_message(desc_v, current_query, mess_for_vali_l, 1);
            }
            free(current_query);
        }
        break;

    default:
        debug_printf("PID dzidziusia: %d\n", child_pid);
        while (true) {
            buffer_cleaner(receive_v, rec_v_size);
            if (rcd == to_receive) {
                debug_printf("TESTERp HAS ALL IT WANTS. ALL OF IT.\n");
                break;
            }
            debug_printf("TESTERp WANTS MORE: %d\n", (to_receive - rcd));
            debug_printf("TESTERpar (pid: %d) shall be reading\n", pid);
            read_message(rec_v, receive_v, rec_v_size);
            debug_printf("Tester%s czyta: %s\n", (is_a_child ? "kid" : ""), receive_v);
            //rcd++;
            debug_printf("RCD: %d\nTO RECEIVE: %d\n", rcd, to_receive);
            if (receive_v[0] == '!') {
                debug_printf("TESTERpar (pid: %d) received !\n", pid);
                sscanf(receive_v, "%*c %d", &to_receive);
            } else {
                debug_printf("TESTERpar (pid: %d) received word\n", pid);
                rcd++;
                char is_accepted;
                char rec_word[MAXLEN + 3];
                rec_word[0] = '\0';
                if (receive_v[2] != '\n') {
                    sscanf(receive_v, "%c %s", &is_accepted, rec_word);
                } else {
                    sscanf(receive_v, "%c", &is_accepted);
                    rec_word[0] = '\0';
                }
                if (is_accepted == 'A') {
                    acc++;
                }
                printf("%s %c\n", rec_word, is_accepted);
            }
        }
        debug_printf("TESTERPAR had no problem with reading from offspring\n");
        report_yourself();
        debug_printf("TESTERpar (pid: %d) shall be cleaning\n", pid);
        cleaner(0);
        debug_printf("TESTERpar cleaned\n");
    }
}

int main() {
    test();
    return 0;
}