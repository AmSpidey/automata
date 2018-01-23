#include <sys/stat.h>
#include "head.h"

mqd_t rec_v;

pid_t pid = 0;

Automaton toReturn;

void basic_prop() {
    scanf("%d %d %d %d %d ",
          &toReturn.line_counter,
          &toReturn.alpha_size,
          &toReturn.states_count,
          &toReturn.universals,
          &toReturn.acc_size);
}

void beginning_state() {
    scanf("%d ", &(toReturn.beginning));
}

void states() {
    int beg = 0;
    char alph = 'a';
    int end = 0;
    char endl;
    for (int z = 0; z < max_state_count; z++) {
        for (int a = 0; a < 'z' - 'a'; a++) {
            toReturn.autom[z][a][0] = 0;
        }
    }
    for (int i = 0; i < toReturn.line_counter - 3; i++) {
        scanf("%d %c", &beg, &alph);
        endl = 'a';
        while (endl != '\n') {
            scanf("%d%c", &end, &endl);
            toReturn.autom[beg][alph - 'a'][0]++;
            toReturn.autom[beg][alph - 'a'][toReturn.autom[beg][alph - 'a'][0]] = end;
        }
    }
}


void accepting_states() {
    int accepted = 0;
    for (int i = 0; i < max_state_count; i++) {
        toReturn.accepted[i] = false;
    }
    for (int i = 0; i < toReturn.acc_size; i++) {
        scanf("%d", &accepted);
        toReturn.accepted[accepted] = true;
    }
}


void read_aut() {
    basic_prop();
    beginning_state();
    accepting_states();
    states();
}

typedef struct testers_list {
    int pid;
    int rcd;
    int acc;
    struct testers_list* next;
    char* q_name;
    mqd_t desc_t;
} TestersList;

typedef struct valirasputins {
    int sent_queries;
    int received_resp;
    int accepted_words;
    TestersList* testers;
    TestersList* latest;
} Valirasputins;


Valirasputins vali;

TestersList* find_after_pid(int pid) {
    TestersList* iter = vali.testers;
    while (iter) {
        if (iter->pid == pid) return iter;
        iter = iter->next;
    }
    return iter;
}

void destroy_tester(TestersList* temp) {
    if (mq_close(temp->desc_t) == -1) {
        system_err_noclean("Error in closin mq");
    }
    free(temp->q_name);
    free(temp);
}

void cleaner(bool err) {
    debug_printf("Validator cleaning up:\n");
    TestersList* iter = vali.testers;
    while (iter) {
        TestersList* temp = iter;
        iter = iter->next;
        if (err) {
            kill(temp->pid, SIGINT);
        }
        destroy_tester(temp);
    }
    if (mq_unlink(link_rec_v) == -1) {
        system_err_noclean("Error in unlinking mq");
    } else {
        debug_printf("Validator removed his mq\n");
    }
    if (mq_close(rec_v) == -1) {
        system_err_noclean("Error in closin mq");
    }
}

int exclam(char* mess, TestersList* test) {
    return sprintf(mess, "! %d", test->rcd);
}

TestersList* newTester(int pid) {
    TestersList* new = malloc(sizeof(TestersList));
    new->rcd = 0;
    new->acc = 0;
    new->pid = pid;
    new->next = NULL;
    char* new_q = calloc(20, sizeof(char));
    link_v_t_pid(new_q, new->pid);
    new->q_name = new_q;
    debug_printf("Opening mqueue called \"%s\"for tester@%d\n", new->q_name, new->pid);
    open_m_que(new->q_name, &(new->desc_t));
    if (vali.latest) {
        vali.latest->next = new;
    }
    vali.latest = new;
    if (!vali.testers) vali.testers = new;
    return new;
}

TestersList* create_run_argument(char* mess, char* word) {
    int pid = 0;
    if (mess[0] != '\n') {
        sscanf(mess, "%s %d", word, &pid);
    } else {
        sscanf(mess, "%d", &pid);
        sprintf(word, "\n");
    }
    TestersList* tester = find_after_pid(pid);
    if (!tester) {
        tester = newTester(pid);
    }
    tester->rcd++;
    return tester;
}

void new_validator() {
    vali.latest = NULL;
    vali.testers = NULL;
    vali.received_resp = 0;
    vali.accepted_words = 0;
    vali.sent_queries = 0;
}

void cleaner_callback(int sig) {
    fprintf(stderr, "Received signal %s, cleaning up.\n", strsignal(sig));
    cleaner(sig != SIGUSR1);
    exit(-1);
}

void cleaner_when_sigint() {
    struct sigaction action;
    action.sa_handler = cleaner_callback;

    sigset_t mask;
    if (sigemptyset(&mask)) {
        system_err("error in sigemptyset");
    }
    if (sigaddset(&mask, SIGINT)) {
        system_err("error in sigaddset");
    }
    action.sa_mask = mask;
    action.sa_flags = 0;
    if (sigaction(SIGINT, &action, 0) == -1) {
        system_err("error in sigaction");
    }
}

void report_yourself() {
    printf("Rcd: %d\nSnt: %d\nAcc: %d\n", vali.received_resp,
           vali.sent_queries, vali.accepted_words);
    for (TestersList* iter = vali.testers; iter; iter = iter->next) {
        printf("PID: %d\nRcd: %d\nAcc: %d\n",
               iter->pid, iter->rcd, iter->acc);
    }
}

int child_pid = 0;

void validate() {
    pid = getpid();
    read_aut();
    cleaner_when_sigint();
    new_validator();

    struct mq_attr mq_v;
    create_m_que(link_rec_v, &rec_v);
    int buff_size_v = buffer_size(rec_v, &mq_v);
    char buff_v[buff_size_v];

    int fildes[2];
    if (pipe(fildes) == -1) {
        system_err("Error in pipe! Oopsie-doopsie\n");
    }
    debug_printf("Ive read the automaton hehe\n");

    bool was_exclaimed = false;
    char pid_char[20];

    do {
        debug_printf("Now reading from mqueue\n");
        buffer_cleaner(buff_v, buff_size_v);
        read_message(rec_v, buff_v, buff_size_v);
        if (is_a_num(buff_v[0])) {
            debug_printf("IT'S A MESSAGE FROM RUN :)\n");
            vali.received_resp++;
            char mess[1000 + 2 + 13];
            char word[1000];
            word[0] = '\0';
            int tester_pid = -666;
            int czy_ok = 0;
            sscanf(buff_v, "%d %d %s", &tester_pid, &czy_ok, word);
            TestersList* tester = find_after_pid(tester_pid);
            char yes_no = 'N';
            if (czy_ok) {
                yes_no = 'A';
                tester->acc++;
                vali.accepted_words++;
            }
            int len = sprintf(mess, "%c %s ", yes_no, word);

            write_message(tester->desc_t, mess, len, 1);
        } else {
            char word[1000];
            word[0] = '\0';

            if (was_exclaimed) {
                debug_printf("Request after termination: %s\n", buff_v);
                int pid = 0;
                if (buff_v[0] != '\n') {
                    sscanf(buff_v, "%s %d", word, &pid);
                } else {
                    sscanf(buff_v, "%d", &pid);
                    sprintf(word, "\n");
                }
                TestersList* tester = find_after_pid(pid);
                if (!tester) {
                    tester = newTester(pid);
                    write_message(tester->desc_t, "! 0", 3, 1);
                }
            } else if (buff_v[0] == '!') {
                debug_printf("It's a termination!\n");
                was_exclaimed = true;
                for (TestersList* iter = vali.testers; iter; iter = iter->next) {
                    char mess[11];
                    int length_it = exclam(mess, iter);
                    write_message(iter->desc_t, mess, length_it, 2);
                }
            } else {
                debug_printf("It's a regular ol' request!\n");
                vali.sent_queries++;
                create_run_argument(buff_v, word);

                debug_printf("Czeba by runa zrobić do %s\n", buff_v);

                switch (child_pid = fork()) {
                case -1:
                    system_err("Error in fork\n");

                case 0:
                    debug_printf("Jestem dzidzią vali a mój pid to %d. Forknęło się.\n", getpid());
                    if (close(fildes[1]) == -1) {
                        system_err("Error in closing pipe\n");
                    }
                    if (dup2(fildes[0], 0) == -1) {
                        system_err("Failure in duplicating descriptor\n");
                    }
                    sprintf(pid_char, "%d", pid);
                    execlp("./run", "./run", buff_v, pid_char, NULL);
                    system_err("Failure in execlp\n");
                    break;
                default:
                    break;
                }

                PRINT_ERR(-1 == write(fildes[1], (char*) (&toReturn), sizeof(Automaton)), "Error in writing to pipe");
                debug_printf("Sent the automaton to the run\n");
            }
        }
    } while (!was_exclaimed || vali.received_resp != vali.sent_queries);
    report_yourself();
    cleaner(errno);
    if (errno) {
        exit(-1);
    }
}

int main() {
    validate();
    return 0;
}