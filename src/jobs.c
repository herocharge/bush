#include "../include/jobs.h"

Job create_job(int is_fg, int stat, char* cmd){
    Job job;
    job.is_fg = is_fg;
    job.stat = stat;
    job.cmd = (char *)strdup(cmd);
    return job;
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