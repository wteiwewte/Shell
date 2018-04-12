#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "background.h"

volatile int foreground_childs;
volatile arr_status background_childs;
volatile pid_t arr_foreground[FOREGROUND_SIZE];

void handler_sigchld(int sig)
{
    /*
        blocking sigchld so we won't get one durign execution of our handler
    */
    sigset_t blocking;
    sigemptyset(&blocking);
    sigaddset(&blocking, SIGCHLD);
    sigprocmask(SIG_BLOCK, &blocking, NULL);

    pid_t p;
    int status;
    /*
        while checking if there is any child zombie
    */
    while( (p=waitpid(-1, &status, WNOHANG)) > 0 ){
        if( get_index(background_childs.id, p, BACKGROUND_SIZE) == NOT_FOUND_THIS_PID_IN_BACKGROUND ) {
            if( get_index(arr_foreground, p, FOREGROUND_SIZE) >= 0 ) {
                foreground_childs--;
                del_pid(arr_foreground, p, FOREGROUND_SIZE);
            }
        }
        else {
            set_status(p, status);
            del_pid(background_childs.id, p, BACKGROUND_SIZE);
        }
    }

    sigprocmask(SIG_UNBLOCK, &blocking, NULL);
}

void initiate(volatile pid_t* tab, int SIZE){
    /*
        setting our arr_status
    */
    int i;
    for( i = 0; i < SIZE; ++i ) tab[i] = NO_STATUS;
}

int get_index(volatile pid_t* tab, pid_t p, int SIZE){
    /*
        function returning pid's index in our statuses array
    */
    int i = 0;
    while( i < SIZE ){
        if( tab[i] == p ) return i;
        i++;
    }
    return NOT_FOUND_THIS_PID_IN_BACKGROUND;
}

void add_pid(volatile pid_t* tab, pid_t p, int SIZE){
    /*
        function adding pid to statuses array
    */
    int i = 0;
    while( tab[i] != NO_STATUS ){
        i++;
    }
    tab[i] = p;
}

void del_pid(volatile pid_t* tab, pid_t p, int SIZE){
    /*
        function deleting pid to statuses array
    */
    if( SIZE == FOREGROUND_SIZE ) tab[get_index(tab, p, SIZE)] = NO_STATUS;
    else tab[get_index(tab, p, SIZE)] = GOT_STATUS;
}

bool is_full(){
    /*
        function checking if our statuses array is full
    */
    int i = 0;
    while( i < BACKGROUND_SIZE ){
        if( background_childs.id[i] == NO_STATUS ) return false;
        i++;
    }
    return true;
}

void set_status(pid_t p, int status){
    /*
        function setting pid's status in our statuses array
    */
    int id_pid = get_index(background_childs.id, p, BACKGROUND_SIZE);
    if( id_pid >= 0 ){
        if( WIFEXITED(status) ) {
            sprintf((char *) background_childs.status[id_pid], "Background process %d terminated. (exited with status %d)\n", background_childs.id[id_pid], WEXITSTATUS(status));
        }
        else if( WIFSIGNALED(status) ){
            sprintf((char *) background_childs.status[id_pid], "Background process %d terminated. (killed by signal %d)\n", background_childs.id[id_pid], WTERMSIG(status));
        }
    }
}

void print_statuses(){
    /*
        function printing all available statuses
    */
    int i;
    for( i = 0; i < BACKGROUND_SIZE; ++i ) {
        if( background_childs.id[i] == GOT_STATUS ){
            printf("%s\n", background_childs.status[i]);
            background_childs.id[i] = NO_STATUS;
        }
    }
    fflush(stdout);
}
