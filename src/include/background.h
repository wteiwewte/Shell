#ifndef _BACKGROUND_H_

#define _BACKGROUND_H_

#include <stdbool.h>
#include <unistd.h>
#define BACKGROUND_SIZE 1024
#define FOREGROUND_SIZE 2048
#define NOT_FOUND_THIS_PID_IN_BACKGROUND -1
#define GOT_STATUS -2
#define NO_STATUS -1

typedef struct arr_status {
    volatile char status[BACKGROUND_SIZE][100];
    volatile pid_t id[BACKGROUND_SIZE];
} arr_status;

//number of processes in foreground
extern volatile int foreground_childs;
//structure to storing pids and statuses from background processes
extern volatile arr_status background_childs;

//structure to storing pids childs in foreground
extern volatile pid_t arr_foreground[FOREGROUND_SIZE];

void handler_sigchld(int);
void initiate(volatile pid_t*, int);
bool is_full();
int get_index(volatile pid_t*, pid_t, int);
void add_pid(volatile pid_t*, pid_t, int);
void del_pid(volatile pid_t*, pid_t, int);
void set_status(pid_t, int);
void print_statuses();

#endif // _BACKGROUND_H_


