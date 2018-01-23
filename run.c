#include "head.h"

mqd_t desc_v;
pid_t validator_pid = 0;
Automaton automat;
bool mq_is_open = false;

bool accept(char* w, int state) {
    DEBUG_PRINTF_ERR("PID: %d is starting to accept word \"%s\" in state %d: \n", getpid(), w, state);
    if (!w[0]) {
        DEBUG_PRINTF_ERR("End of the wooord %d %d\n", getpid(), automat.accepted[state]);
        return (automat.accepted[state]);
    }
    int states_to_get = automat.autom[state][w[0] - 'a'][0];
    int fildes[2];
    int result = 0;
    pid_t mother_pid;

    if (states_to_get == 0) {
        DEBUG_PRINTF_ERR("PID: %d nie ma gdzie iść\n", mother_pid);
        return (state < automat.universals);
    }

    PRINT_ERR(pipe(fildes) == -1, "Error in pipe! Oopsie-doopsie\n");

    int accept_no = 0;

    for (int i = 0; i < states_to_get - 1; i++) {
        DEBUG_PRINTF_ERR("Let's check out this state: %d : pid %d\n", i, mother_pid);
        switch (fork()) {
        case -1:
            system_err("Error in fork\n");
            break;
        case 0: //child
            PRINT_ERR(close(fildes[0]) == -1, "Error in closing pipe\n");
            DEBUG_PRINTF_ERR("Jestem dzidziusiem runa o pidzie: %d Mój pid: %d\n", mother_pid, getpid());
            accept_no = accept(w + 1, automat.autom[state][w[0] - 'a'][i + 1]);
            DEBUG_PRINTF_ERR(accept_no ? "accepted %d %s\n" : "not_accepted %d %s\n", getpid(), w);

            if (write(fildes[1], accept_no ? "1 " : "0 ", 2) == -1) {
                system_err("Error in writing to pipe\n");
            }
            DEBUG_PRINTF_ERR("Dziecko@%d runa@%d pomyślnie zakończyło\n", getpid(), mother_pid);
            exit(0);
        default:
            break;
        }

    }
    DEBUG_PRINTF_ERR("PID: %d będzie zaraz gromadził\n", mother_pid);

    accept_no += accept(w + 1, automat.autom[state][w[0] - 'a'][states_to_get]);
    DEBUG_PRINTF_ERR(accept_no == 1 ? "accepted %d %s\n" : "not_accepted %d %s\n", mother_pid, w);
    for (int i = 0; i < states_to_get - 1; i++) {
        int wait_status = 0;
        DEBUG_PRINTF_ERR("PID: %d będzie zaraz gromadził state %d\n", mother_pid, i);
        wait(&wait_status);
        if (!WIFEXITED(wait_status) || WEXITSTATUS(wait_status) != 0) {
            system_err("Error in child of run\n");
        }
        const int BSIZE = 2;
        char buf[BSIZE];
        PRINT_ERR((-1 == read(fildes[0], buf, BSIZE)), "Error in read from pipe\n");
        sscanf(buf, "%d", &result);
        accept_no += result;
    }

    if (close(fildes[1]) == -1) {
        system_err("Error in closing pipe\n");
    }
    close(fildes[0]);
    if (state >= automat.universals) { //egzystencjalny
        return (accept_no > 0);
    } else {
        return (accept_no == states_to_get);
    }
}

void cleaner(bool err) {
    if (mq_is_open && mq_close(desc_v) == -1) {
        system_err_noclean("Error in closing mq\n");
    }
    if (err) {
        kill(validator_pid, SIGINT);
    }
}


void cleaning(int signal) {
    cleaner(signal != 0);
    exit(-1);
}


int main(int argc, char* argv[]) {
    validator_pid = strtol(argv[2], 0, 0);
    if (argc != 3) {
        system_err("Bad run call :(");
    }
    if (DEBUG) {
        char run_oucik[25];
        sprintf(run_oucik, "run_out_%d.out", getpid());
        freopen(run_oucik, "w", stderr);
    }
    DEBUG_PRINTF_ERR("Zrobiłem cokolwiek\n");
    signal(SIGUSR1, cleaning);
    if (prctl(PR_SET_PDEATHSIG, SIGUSR1) < 0)
        system_err("prctl\n");
    if (read(0, (char*) (&automat), sizeof(Automaton)) == -1) {
        system_err("Failure to read automaton\n");
    }
    DEBUG_PRINTF_ERR("I've read the automaton\n");
    int tester_pid = 0;
    DEBUG_PRINTF_ERR("Zaraz machnę i będę scanfować\n");
    char word[1000];
    bool is_ok = false;
    if (argv[1][0] != '\n') {
        sscanf(argv[1], "%s %d", word, &tester_pid);
        is_ok = accept(word, automat.beginning);
    } else {
        sprintf(word, "\n");
        sscanf(argv[1], "%d", &tester_pid);
        is_ok = accept("", automat.beginning);
    }
    struct mq_attr mq_v;

    open_m_que(link_rec_v, &desc_v);
    mq_is_open = true;
    int send_v_size = buffer_size(desc_v, &mq_v);
    char send_v[send_v_size];
    int len = sprintf(send_v, "%d %d %s", tester_pid, is_ok, word);
    DEBUG_PRINTF_ERR(is_ok == 1 ? "WHOLE WORD accepted %d \"%s\"\n" : "WHOLE WORD not_accepted %d %s\n", getpid(), word);
    write_message(desc_v, send_v, len, 1);
    cleaner(0);
}

