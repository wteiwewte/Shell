#include "config.h"
#include "siparse.h"
#include "utils.h"
#include "builtins.h"
#include "buffers.h"
#include "pipes.h"
#include "background.h"

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    //initialization our variables
    int len;
    offset = 0;
    foreground_childs = 0;
    skip_the_next_line = false;
    line_pack first_line;
    initiate(background_childs.id, BACKGROUND_SIZE);
    initiate(arr_foreground, FOREGROUND_SIZE);

    /*
        we're blocking sigchld, so we can handle it in future
    */

    struct sigaction sa;
    sa.sa_handler = handler_sigchld;
    sigaction(SIGCHLD, &sa, NULL);

    while( (len = read(0, buf + offset, 2*MAX_LINE_LENGTH-offset)) != 0 ){
        if( check_prompt() ) print_prompt();
        if( len == ERROR_READ ) continue;
        offset += len;
        first_line = get_first_line(offset);

        if( first_line.info == GOT_EOL ){
            /*
                we got whole line with EOF
                so we can proceed with our line
            */
            process_line(first_line);
            moving_bytes(first_line);
        }
        else if( first_line.info == NOT_GOT_EOL ) {
            //we got part of line without EOL
            if( is_line_too_long(offset) ) {
                /*
                    if this part is already too long
                    we want to skip the next part,
                    so we setting our bool to true
                */
                skip_the_next_line = true;
                /*
                    and we are moving bytes (we know we won't use it because of line's length)
                */
                moving_bytes(first_line);
            }
        }
    }
    while( offset > 0 ){
        first_line = get_first_line(offset);
        moving_bytes(first_line);
        process_line(first_line);
    }

}

















