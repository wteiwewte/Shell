#ifndef _PIPES_H_
#define _PIPES_H_

#include "siparse.h"
#include <stdbool.h>

#define FAULTYREDIRS false
#define CORRECTREDIRS true

void exec_line(line* l);
void exec_pipeline(pipeline, bool);
void exec_command(int, int, command*, bool);
bool handle_redirs(command*);
bool IS_NULL_COMMAND(command*);
bool IS_VALID_PIPELINE(pipeline);
bool IS_FIRST_COMMAND_IN_PIPELINE(int);
bool IS_NOT_FIRST_COMMAND_IN_PIPELINE(int);
bool IS_LAST_COMMAND_IN_PIPELINE(int, pipeline);
bool IS_NOT_LAST_COMMAND_IN_PIPELINE(int, pipeline);
int len_pipe(pipeline);

#endif // _PIPES_H_
