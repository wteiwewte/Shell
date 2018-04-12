#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <stddef.h>
#include "config.h"
#include "builtins.h"
#include "buffers.h"
#include "pipes.h"
#include "background.h"
#include "utils.h"

void exec_command(int inFD, int outFD, command* com, bool is_background){
    if( IS_NULL_COMMAND(com) ) return;
    /*
        blocking sigchld so we won't increment already dead child process
    */
    sigset_t u;
    sigemptyset(&u);
    sigaddset(&u, SIGCHLD);
    sigprocmask(SIG_BLOCK, &u, NULL);

    pid_t child_pid;
    if( (child_pid = fork()) == 0 ){
        if( is_background ){
            //creating own group
            setsid();
            //setting default sigint's service
            signal(SIGINT, SIG_DFL);
        }

        /*
            possibly redirect to file descriptors from pipes
        */
        if( inFD != 0 ) {
            dup2(inFD, 0);
            close(inFD);
        }

        if( outFD != 1 ){
            dup2(outFD, 1);
            close(outFD);
        }
        /*
            but after check normal redirections
        */
        if( handle_redirs(com) == CORRECTREDIRS ){
            if( execvp(com->argv[0],com->argv) == -1 ){
                handle_command_error(com);
                exit(EXEC_FAILURE);
            }
        }
        else {
            exit(EXEC_FAILURE);
        }
    }
    else{
        if( is_background ){
            if( !is_full() ) add_pid(background_childs.id, child_pid, BACKGROUND_SIZE);
        }
        else{
            add_pid(arr_foreground, child_pid, FOREGROUND_SIZE);
            foreground_childs++;
        }
    }
    sigprocmask(SIG_UNBLOCK,&u, NULL);
}

void exec_line(line* l){
    if( l ){
        if( l->flags == LINBACKGROUND ){
            exec_pipeline(l->pipelines[0], true);
        }
        else {
            int idPipeline = 0;
            while( l->pipelines[idPipeline] ){
                exec_pipeline(l->pipelines[idPipeline], false);
                idPipeline++;
            }
        }
    }
    else {
        // if parsing was failure printing syntax error
        print_syntax_err();
    }
}


void exec_pipeline(pipeline p, bool is_background){
    if( !IS_VALID_PIPELINE(p) ) {
        print_syntax_err();
        return;
    }
    /*
        blocking sigchld during execution of pipeline
    */
    sigset_t v;
    sigemptyset(&v);
    sigaddset(&v, SIGCHLD);
    sigprocmask(SIG_BLOCK, &v, NULL);

    int len_pipeline = len_pipe(p);
    int pipes_fd[2*(len_pipeline-1)];

    if( len_pipeline > 1 ){
        /*
            iterating through our pipeline connecting consecutive commands with pipes
        */
        int pd[2];
        int inFD = 0;
        int id_command;
        for( id_command = 0; id_command < len_pipeline ; ++id_command ){
            if( IS_NOT_LAST_COMMAND_IN_PIPELINE(id_command, p) ) {
                    pipe(pd);
                    fcntl(pd[0], F_SETFD, FD_CLOEXEC);
                    fcntl(pd[1], F_SETFD, FD_CLOEXEC);
                    pipes_fd[2*id_command] = pd[0];
                    pipes_fd[2*id_command+1] = pd[1];
            }
            if( IS_LAST_COMMAND_IN_PIPELINE(id_command, p) ) {
                exec_command(inFD, 1, p[id_command], is_background);
                close(pipes_fd[2*(id_command-1)]);
            }
            else {
                exec_command(inFD, pd[1], p[id_command], is_background);
                if( IS_NOT_FIRST_COMMAND_IN_PIPELINE(id_command) ) close(pipes_fd[2*(id_command-1)]);
                close(pd[1]);
                inFD = pd[0];
            }
        }

    }
    else if( len_pipeline == 1 ) {
        /*
            if our pipeline is just single command
            we're checking if this is shell direct or just normal command
        */
        command* com = p[0];
        if( if_shell_direct(com) >= 0 ){
            if((*builtins_table[if_shell_direct(com)].fun)(com->argv) == BUILTIN_ERROR ) print_builtin_error(com->argv[0]);
        }
        else {
            exec_command(0, 1, com, is_background);
        }
    }
    if( !is_background ){
        sigset_t y;
        sigemptyset(&y);
        /*
            if there is foreground child we're waiting for sigchld from it
            and eventually checking for any zombies
        */
        while( foreground_childs > 0 ){
            sigsuspend(&y);
        }
    }

    sigprocmask(SIG_UNBLOCK, &v, NULL);


}

bool handle_redirs(command* com){
    int id_redirect = 0;

    while( com->redirs[id_redirect] != NULL ){
        /*
            iterating through all redirections
        */
        int fd;
        if( IS_RIN(com->redirs[id_redirect]->flags) ){
            close(0);
            fd = open(com->redirs[id_redirect]->filename, O_RDONLY, S_IRUSR | S_IWUSR);
            if( fd != 0 ){
                switch(errno){
                    case ENOENT:
                        fprintf( stderr, "%s: no such file or directory\n", com->redirs[id_redirect]->filename);
                        fflush(stderr);
                        return FAULTYREDIRS;
                    case EACCES:
                        fprintf( stderr, "%s: permission denied\n", com->redirs[id_redirect]->filename);
                        fflush(stderr);
                        return FAULTYREDIRS;
                }
            }
        }
        else {
            close(1);
            if( IS_RAPPEND(com->redirs[id_redirect]->flags) ) {
                fd = open(com->redirs[id_redirect]->filename, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
            }
            else {
                fd = open(com->redirs[id_redirect]->filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            }
            if( fd != 1 ){
                switch(errno){
                    case EACCES:
                        fprintf( stderr, "%s: permission denied\n", com->redirs[id_redirect]->filename);
                        fflush(stderr);
                        return FAULTYREDIRS;
                }
            }
        }

        id_redirect++;
    }
    return CORRECTREDIRS;
}

bool IS_NULL_COMMAND(command* com){
    return (com == NULL || com->argv[0] == NULL);
}

bool IS_VALID_PIPELINE(pipeline p){
    if( len_pipe(p) >= 2 ){
        /*
            CHECKING IF PIPE CONTAINS NULL COMMAND
        */
        int id_command = 0;
        while( p[id_command] ){
            if( IS_NULL_COMMAND( p[id_command] ) ) return false;
            id_command++;
        }
    }
    return true;
}

bool IS_FIRST_COMMAND_IN_PIPELINE(int id_command){
    return (id_command == 0);
}

bool IS_NOT_FIRST_COMMAND_IN_PIPELINE(int id_command){
    return (id_command > 0);
}

bool IS_LAST_COMMAND_IN_PIPELINE(int id_command, pipeline p){
    return (id_command == len_pipe(p) - 1);
}

bool IS_NOT_LAST_COMMAND_IN_PIPELINE(int id_command, pipeline p){
    return !(IS_LAST_COMMAND_IN_PIPELINE(id_command, p));
}

int len_pipe(pipeline p){
    int id_command= 0;

    while( p[id_command] != NULL ){
        id_command++;
    }
    return id_command;
}
