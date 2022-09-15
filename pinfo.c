#include "pinfo.h"

void pinfo(int argc, int pid){
    char pid_s[10];
    sprintf(pid_s, "%d", pid);
    char* path = (char *)malloc(PATH_MAX+1);
    strcpy(path, "/proc/");
    strcat(path, pid_s);
    strcat(path, "/stat");
    FILE* stat = fopen(path, "r");
    if(stat == NULL){
        printf("process with pid %d doesnt exist\n", pid);
        return;
    }
    int pid_n, ppid, pgrp, session, tty_nr, tpgid;
    unsigned int dummy;
    unsigned long dumm2, vmsize;
    long int dumm3;
    char exec_path[PATH_MAX + 1];
    char state;
    unsigned long long dumm4;
    fscanf(stat, "%d %s %c %d %d %d %d %d %u", &pid_n, exec_path, &state, &ppid, &pgrp, &session, &tty_nr, &tpgid, &dummy);
    for(int i = 10; i<= 15;i++)fscanf(stat, "%lu", &dumm2);
    for(int i = 16; i<= 21;i++)fscanf(stat, "%ld", &dumm3);
    fscanf(stat, "%llu %lu", &dumm4, &vmsize);
    printf("pid: %d\n", pid_n);
    printf("process status: %c", state);
    if(pgrp == tpgid){
        printf("+");
    }printf("\n");
    printf("memory: %lu\n", vmsize);
    fclose(stat);
    char path2[PATH_MAX];
    strcpy(path2, "/proc/");
    strcat(path2, pid_s);
    strcat(path2, "/exe");
    int len = readlink(path2, exec_path, PATH_MAX);
    if(len == -1){
        perror(path2);
        return;
    }
    exec_path[len] = 0;
    printf("executable path: %s\n", exec_path);
    
}
