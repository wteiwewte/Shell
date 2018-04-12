#ifndef _BUFFERS_H_
#define _BUFFERS_H_

#include <stdbool.h>

#include "siparse.h"
#include "config.h"


#define NOT_GOT_EOL -1
#define GOT_EOL -2
#define ERROR_READ -1

//bool checking if we got earlier part of line and this part of line was already too long
extern bool skip_the_next_line;
//buff
extern char buf[2*MAX_LINE_LENGTH+1];
//our buff's offset
extern int offset;

typedef struct line_pack {
    line* l;
    int len;
    int info;
} line_pack;

line_pack get_first_line(int);
bool is_line_too_long(int);
bool check_prompt();
void print_prompt();
void handle_command_error();
void print_syntax_err();
void process_line(line_pack);
void moving_bytes(line_pack);
#endif /* !_BUFFERS_H_ */
