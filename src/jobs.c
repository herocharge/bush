#include "../include/jobs.h"

Job create_job(int is_fg, int stat, char* cmd){
    Job job;
    job.is_fg = is_fg;
    job.stat = stat;
    job.cmd = (char *)strdup(cmd);
    return job;
}

char* statstr(int stat){
    switch (stat){
        case 0: return strdup("Done   ");
        case 1: return strdup("Stopped");
        case 2: return strdup("Running");
        case 3: return strdup("Ready  ");
        case 4: return strdup("Killed ");
        default: return strdup("?");
    }
}

int kill_job(Job* jobs, int ind){
    if(kill(-jobs[ind].pid, SIGKILL) == -1)return -1;
    free_job(jobs[ind]);
    jobs[ind].stat = DONE;
    return 0;
}


void free_job(Job job){
    free(job.cmd);
}