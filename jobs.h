#ifndef JOBS_H
#define JOBS_H

#include <unistd.h>
#include <sys/types.h>

enum status{
    DONE = 0,
    STOPPED = 1,
    RUNNING = 2
};

typedef struct job{
    pid_t pid;  // process_id
    pid_t pgid; // process group id
    int is_fg;  // is foreground process ?
    int status; // done? stopped? running?
}Job;


#endif