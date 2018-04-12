#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <stdbool.h>

#include "builtins.h"
#include "siparse.h"
#include "pipes.h"

builtin_pair builtins_table[]={
	{"exit",	&lexit},
	{"lecho",	&echo},
	{"lcd",		&lcd},
	{"lkill",	&lkill},
	{"lls",		&lls},
	{NULL,NULL}
};


int if_shell_direct(command* com){
    /*
        function checking if command is shell direct
        and returning her index in case it is otherwise -1
    */
    if( IS_NULL_COMMAND(com) ) return -1;
    int i = 0;
    while(builtins_table[i].name != NULL){
        if( strcmp(com->argv[0],builtins_table[i].name) == 0 ) return i;
        i++;
    }
    return -1;
}

bool is_correct_number_args(char* name, char** argv){
    /*
        function checking correct number of arguments passed to shell direct
    */
    int number_of_args = 1;
    while( argv[number_of_args] != NULL ) number_of_args++;
    number_of_args -= 1;
    int i = 0;
    if( strcmp(name, "exit") == 0 ){
        if( number_of_args == 0 ) return CORRECT_ARGS;
        else return BAD_ARGS;
    }
    else if( strcmp(name, "lcd") == 0 ){
        if( number_of_args <= 1 ) return CORRECT_ARGS;
        else return BAD_ARGS;
    }
    else if( strcmp(name, "lkill") == 0 ){
        if( 1 <= number_of_args && number_of_args <= 2 ) return CORRECT_ARGS;
        else return BAD_ARGS;

    }
    else if( strcmp(name, "lls") == 0 ){
        if( number_of_args == 0 ) return CORRECT_ARGS;
        else return BAD_ARGS;
    }
    else return BAD_ARGS;
}


int lexit(char* argv[]){
    exit(0);
}

int echo( char * argv[]){
	int i =1;
	if (argv[i]) printf("%s", argv[i++]);
	while  (argv[i])
		printf(" %s", argv[i++]);

	printf("\n");
	fflush(stdout);
	return 0;
}

int lcd(char* argv[]){
    if( is_correct_number_args(argv[0], argv) == BAD_ARGS ) return BUILTIN_ERROR;

    else if( argv[1] != NULL ){
        /*
            if we got argument we pass it to chdir
        */
        if( chdir(argv[1]) == -1 ){
            return BUILTIN_ERROR;
        }
    }
    else{
        if( chdir(getenv("HOME")) == -1 ){
            return BUILTIN_ERROR;
        }
    }
    return 0;
}

int handle_parse_errors(char *str){
    char *temp;
    int value = strtol(str, &temp, 10);
    if ( *str != '\0' && *temp == '\0') return (value >= 0) ? value : -value;
    else return -1;
}

int lkill(char* argv[]){
    if( is_correct_number_args(argv[0], argv) == BAD_ARGS ) return BUILTIN_ERROR;
    else {
        int signal_nr;
        pid_t pid;
        /*
            first we want to parse numbers from strings
        */
        if( argv[2] != NULL ){
            if( (signal_nr = handle_parse_errors(argv[1])) == PARSE_ERROR ) return BUILTIN_ERROR;
            if( (pid = handle_parse_errors(argv[2])) == PARSE_ERROR ) return BUILTIN_ERROR;
        }
        else {
            signal_nr = 15;
            if( (pid = handle_parse_errors(argv[1])) == PARSE_ERROR ) return BUILTIN_ERROR;
        }
        /*
            then invoke kill function
        */
        if( kill(pid, signal_nr) == -1 ){
            return BUILTIN_ERROR;
        }
    }
    return 0;
}



int lls(char* argv[]){
    if( is_correct_number_args(argv[0], argv) == BAD_ARGS ){
        return BUILTIN_ERROR;
    }
    DIR* my_dir;
    struct dirent* my_file;
    char dir_name[1024];
    getcwd(dir_name,sizeof(dir_name));
    my_dir = opendir(dir_name);
    if( my_dir == NULL ){
        return BUILTIN_ERROR;
    }
    else{
        /*
            reading all files from dir
        */
        while((my_file = readdir(my_dir)) != NULL){
            if( my_file->d_name[0] != '.' ) printf("%s\n",my_file->d_name);
        }
    }
    closedir(my_dir);
    fflush(stdout);
    return 0;
}


int undefined(char * argv[]){
	fprintf(stderr, "Command %s undefined.\n", argv[0]);
	return BUILTIN_ERROR;
}

void print_builtin_error(char* name){
    fprintf( stderr, "Builtin %s error.\n", name);
    fflush(stderr);
}
