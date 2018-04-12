#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "siparse.h"
#include "builtins.h"
#include "config.h"
#include "buffers.h"
#include "pipes.h"
#include "background.h"

bool skip_the_next_line;
char buf[2*MAX_LINE_LENGTH+1];
int offset;

line_pack get_first_line(int len){
    /*
        func to get first line from our buf
    */
    int i = 0;
    line_pack to_return;
    bool got_EOL = true;

    //cutting out first line from our buf
    while( buf[i] != '\n' ){
        if( i == len - 1 ){
            got_EOL = false;
            break;
        }
        i++;
    }

    char gotten_bytes[i+1];
    memmove(gotten_bytes, buf, i);
    gotten_bytes[i] = '\0';
    if( !is_line_too_long(i) ) to_return.l = parseline(gotten_bytes);
    else to_return.l = NULL;
    to_return.len = i+1;
    if( got_EOL ) to_return.info = GOT_EOL;
    else to_return.info = NOT_GOT_EOL;

    return to_return;
}

bool is_line_too_long(int len_){
    //checking if line isn't too long
    if ( len_ > MAX_LINE_LENGTH ) return true;
    else return false;
}

void print_prompt(){
    /*
        printing prompt and beforehand all stored statuses of background's childs
    */
    struct stat sth;
    fstat(0, &sth);
    if( S_ISCHR(sth.st_mode) ) {
        print_statuses();
        fprintf( stdout, PROMPT_STR );
        fflush(stdout);
    }
}

bool check_prompt(){
    //checking if STDIN is character device
    struct stat sth;
    fstat(0, &sth);
    if( S_ISCHR(sth.st_mode) ) return true;
    else return false;
}

void handle_command_error(command* com){
    /*
        function to print error from exec our command
    */
    switch (errno){
        case ENOENT:
            fprintf( stderr, "%s: no such file or directory\n", com->argv[0]);
            break;
        case EACCES:
            fprintf( stderr, "%s: permission denied\n", com->argv[0]);
            break;
        default: fprintf( stderr, "%s: exec error\n", com->argv[0]);
    }
}

void print_syntax_err(){
    //printing syntax error
    fprintf( stderr, SYNTAX_ERROR_STR);
    fprintf( stderr, "\n" );
}

void process_line(line_pack l){
    if( skip_the_next_line ){
        /*
            if we got earlier only part of a line,
            and that earlier part was already too long,
            we're skipping the next part
        */
        skip_the_next_line = false;
        /*
            and printing syntax error
        */
        print_syntax_err();
    }
    else if( is_line_too_long(l.len) ) {
        /*
            or if line was too long we're skipping it too
            and printing syntax error
        */
        print_syntax_err();
    }
    else {
        /*
            otherwise we're trying to execute line
        */
        exec_line(l.l);
    }
}

void moving_bytes(line_pack l){
    /*
        moving our offset and buf
        and bytes therefore we can read further bytes from stdin
    */
    offset -= l.len;
    memmove(buf, buf+l.len, 2*MAX_LINE_LENGTH-l.len);
}
