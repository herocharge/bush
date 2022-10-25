#ifndef JOBS_H
#define JOBS_H

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

enum Status{
    DONE = 0,
    STOPPED = 1,
    RUNNING = 2,
    READY = 3,
    KILLED = 4
};

typedef struct job{
    pid_t pid;  // process_id
    pid_t pgid; // process group id
    int is_fg;  // is foreground process ?
    int stat;// done? stopped? running?
    char* cmd;  // command to execute
    int status_printed;
    int exit_code;
    int sig_code;
    int hide;
    FILE* input;
    FILE* output; // fds
} Job;

Job create_job(int is_fg, int stat, char* cmd);
int kill_job(Job* jobs, int ind);
void free_job(Job job);
char* statstr(int stat);

#endif