
#ifndef _BUILTINS_H_
#define _BUILTINS_H_

#define BUILTIN_ERROR 2
#define CORRECT_ARGS true
#define BAD_ARGS false

#define PARSE_ERROR -1
#include "siparse.h"
#include <stdbool.h>

typedef struct {
	char* name;
	int (*fun)(char**);
} builtin_pair;

int echo(char*[]);
int undefined(char*[]);
int lexit(char*[]);
int lcd(char*[]);
int lkill(char*[]);
int lls(char*[]);

extern builtin_pair builtins_table[];
int if_shell_direct(command*);
bool is_correct_number_args(char*, char**);
void print_builtin_error(char*);
int handle_parse_errors(char*);

#endif /* !_BUILTINS_H_ */
