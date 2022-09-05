#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pwd.h>
#include <fcntl.h>
#include <signal.h>

#include "string_utils.h"
#include "echo.h"
#include "cd.h"
#include "jobs.h"
#include "ls.h"
// #include "jobs.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2


#define COMMAND_SIZE 10000
#define CWD_SIZE 10000
#define UNAME_SIZE 10000
#define SYSNAME_SIZE 10000
#define PWD_SIZE 10000
#define MAX_JOBS 1000000
#define HOME_SIZE 10000
// colors
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define COLOR_BOLD  "\e[1m"
#define COLOR_OFF   "\e[m"

// VARS
char OLDPWD[PWD_SIZE];
char HOME[HOME_SIZE];

Job jobs[MAX_JOBS];
int pidtojid[MAX_JOBS];
int number_of_jobs;

void sigchld_handler(int sig, siginfo_t *info, void *ucontext)
{
    pid_t pid;
    int status;
    // printf("sighndlr: %d\n", info->si_code);
    while((pid = waitpid(-1, &status, WNOHANG)) > 0){
        // printf("dead : %d\n", pid);
        // printf("status: %d\n", status);
        // printf("sighndlr: %d\n", info->si_code);
        if(WIFEXITED(status)){
            // printf("done\n");
            jobs[pidtojid[pid]].stat = DONE;
        }
        if(WIFSTOPPED(status)){
            // printf("stopped\n");
            jobs[pidtojid[pid]].stat = STOPPED;
        }
        if(WIFSIGNALED(status)){
            jobs[pidtojid[pid]].stat = KILLED;
        }
        // printf("%d\n", jobs[pidtojid[pid]].stat);
        // printf("jid: %d\n", pidtojid[pid]);
        jobs[pidtojid[pid]].status_printed = 0;
    }
    if(pid == 0){
        if(info->si_status == SIGTTOU){
            jobs[pidtojid[info->si_pid]].stat = STOPPED;
            jobs[pidtojid[info->si_pid]].status_printed = 0;
        }
    }
    //     printf("pid:  %d\n", pid);
    //     printf("status: %d\n", info->si_status);
    // printf("sigchld triggered\n");
}

void sigttin_handler(int sig){
    printf("oh no\n");
}


void print_prompt(){
    char cwd[CWD_SIZE + 1];
    // TODO: make it dynamic
    char sysname[SYSNAME_SIZE + 1];
    char uname[UNAME_SIZE + 1];
    gethostname(sysname, SYSNAME_SIZE);
    struct passwd *pw = getpwuid(getuid());
    strcpy(uname, pw->pw_name);
    char* home_dir = HOME;
    getcwd(cwd, CWD_SIZE);
    int home_dir_len = strlen(home_dir);
    if(!strncmp(cwd, home_dir, home_dir_len)){
        char tmp[CWD_SIZE + 1];
        strcpy(tmp, "~");
        strcpy(&tmp[1], &cwd[home_dir_len]);
        strcpy(cwd, tmp);
    }
    printf("%s%s%s@%s:%s%s$ %s", COLOR_BOLD, KMAG, uname, sysname, KBLU, cwd, COLOR_OFF);
    fflush(stdout);
}

void run(int jobno){
    // printf("Currently running: %s, with pid: %d id fg? %d\n", jobs[jobno].cmd, jobs[jobno].pid, jobs[jobno].is_fg);

    char** argv = NULL;
    int cmdlen = strlen(jobs[jobno].cmd);
    int argc = tokenize(jobs[jobno].cmd, cmdlen, ' ', &argv);
    argv = (char **)realloc(argv, (sizeof(char*))*(argc + 1));
    argv[argc] = NULL;

    if(!strlen(argv[0]))return;
    if(!strcmp(argv[0], "echo")){
            echo(argc, argv);
        }
    else if(!strcmp(argv[0], "cd")){
        if(argc > 2){
            printf("BUSH : cd : Only one arg\n");
            return;
        }
        char tmp[PWD_SIZE];
        getcwd(tmp, PWD_SIZE);
        if(cd(argc, argv, OLDPWD, HOME) == -1)
            printf("Bush : cd : %s : No such directory\n", argv[1]);
        strcpy(OLDPWD, tmp);
    } 
    else if(!strcmp(argv[0], "pwd")){
        char pwd[PWD_SIZE];
        getcwd(pwd, PWD_SIZE);
        printf("%s\n", pwd);
    }  
    else if(!strcmp(argv[0], "jobs")){
        printf("PID \t PGID \t STAT \t CMD\n");
        for(int i =0;i < number_of_jobs;i++){
                if(jobs[i].stat == DONE)continue;
                printf("%d \t %d \t %4d \t %s \n", jobs[i].pid, jobs[i].pgid, jobs[i].stat, jobs[i].cmd);
        }
    }
    else if(!strcmp(argv[0], "ls")){
        // for(int i = 1; i < argc; i++){
        //     printf("%s ", argv[i]);
        // }
        // printf("\n");
        
        ls(argc, argv, HOME);
    }
    else if(!strcmp(argv[0], "fg")){
        int ret = setpgid(atoi(argv[1]), getpgid(getpid()));
        printf("code %d\n", ret);
    }
    else{

        pid_t pid = 0;
        if(jobs[jobno].is_fg){
            pid = fork();
            // printf("Forked...\n");
        } 
        if(!pid){
            if(jobs[jobno].is_fg){
                sigset_t set;
                sigaddset(&set, SIGINT);
                sigprocmask(SIG_UNBLOCK, &set, NULL);
            }
            if(!jobs[jobno].is_fg)
                setpgid(0, 0);
            // if(jobs[jobno].is_fg)
            // tcsetpgrp(STDIN, getpid());
            // printf("%d %d\n", getpid(), getpgid(getpid()));
            execvp(argv[0], argv);
            printf("Bush : %s : Command not found\n", argv[0]);
            kill(getpid(), SIGTERM);
        }
        else{
            // TODO: Handle signals
            // if(jobs[jobno].is_fg)
            //     tcsetpgrp(STDIN, pid);
            int status;
            waitpid(pid, &status, WUNTRACED);
            // tcsetpgrp(STDIN, getpid());
        }
    }
    // destroy_tokens(argv, argc);
}

void run_fg(){
    for(int i = 0; i < number_of_jobs; i++){
        if(jobs[i].is_fg && jobs[i].stat == READY){
            // printf("fg: %s\n", jobs[i].cmd);
            jobs[i].stat = RUNNING;
            jobs[i].pid = getpid();
            jobs[i].pgid = getpgid(jobs[i].pid);
            pidtojid[jobs[i].pid] = i;
            run(i);
            jobs[i].stat = DONE; // Reached for every command
            break;
        }
    }   
}

void run_bg(){
    struct sigaction new_action, old_action;
    new_action.sa_handler = sigchld_handler;
    new_action.sa_flags = 0 | SA_RESTART | SA_SIGINFO;
    sigaction(SIGCHLD, &new_action, &old_action);

    for(int i  = 0; i < number_of_jobs; i++){
        if(!jobs[i].is_fg && jobs[i].stat == READY){
            
            jobs[i].stat = RUNNING;
            pid_t pid = fork();
            if(!pid){
                setpgid(0, 0);
                printf("[%d] %d\n", i, getpid());
                run(i); 
                jobs[i].stat = DONE; // Only reached for builtins
                exit(0);
            }
            else{
                jobs[i].pgid = jobs[i].pid = pid;
                printf("Running %s with pid %d \n", jobs[i].cmd, jobs[i].pid);
                pidtojid[jobs[i].pid] = i;
                int status;
                waitpid(pid, &status, WNOHANG);
            }   
        }
    }
}
void split(char* seq){
    char** cmds;
    int no_of_cmds = tokenize(seq, strlen(seq), '&', &cmds);
    
    // signal(SIGCHLD, sigchld_handler);
    for(int i = 0; i < no_of_cmds - 1; i++){
        // Create a job
        int ind = number_of_jobs;
        // Do better
        // for(int j = 0; j < MAX_JOBS; j++){
        //     if(jobs[j].stat == DONE){
        //         ind = j;
        //         break;
        //     }
        // }
        jobs[ind] = create_job(0, READY, cmds[i]);
        number_of_jobs++;
        // printf("%s\n", jobs[ind].cmd);
        // pid_t pid = fork();
        // pids[i] = pid;
        // if(!pid){
            
        //     // Create child process
        //     setpgid(0, 0);
        //     struct sigaction new_action, old_action;
        //     new_action.sa_handler = sigchld_handler;
        //     sigaction(SIGCHLD, &new_action, &old_action);
        //     run(cmds[i], 1);
        //     kill(getpid(), SIGKILL);
        // }
        // else{
        //     printf("+[%d] pid : %d\n", i+1, pid);
        // }
    }

    // Create the one foreground process
    if(strlen(cmds[no_of_cmds - 1])){
        int ind = number_of_jobs;
        jobs[ind] = create_job(1, READY, cmds[no_of_cmds - 1]);
        number_of_jobs++;
    }
    // printf("%s\n", jobs[ind].cmd);
    // run(cmds[no_of_cmds-1], 0);
    destroy_tokens(cmds, no_of_cmds);
}

void check_done_bg(){
    for(int i = 0; i < number_of_jobs; i++){
        if(jobs[i].stat == DONE && !jobs[i].status_printed && !jobs[i].is_fg){
            printf("[%d]+ Done %s\n", i, jobs[i].cmd);
            jobs[i].status_printed = 1;
        }
        if(jobs[i].stat == STOPPED && !jobs[i].status_printed && !jobs[i].is_fg){
            printf("[%d]+ Stopped %s\n", i, jobs[i].cmd);
            jobs[i].status_printed = 1;
        }
        if(jobs[i].stat == KILLED && !jobs[i].status_printed && !jobs[i].is_fg){
            printf("[%d]+ Killed %s\n", i, jobs[i].cmd);
            jobs[i].status_printed = 1;
        }
    }
}

int main(int argc, char** argv){
    pid_t sess_id = setsid();
    printf("Welcome to Bush -v 0.0.1 \n");
    printf("Bush pid: %d\n", getpid());
    getcwd(HOME, HOME_SIZE);
    
    // Input method 
    FILE* input_file = stdin;
    int shell_mode = 1;
    if(argc > 1){
        shell_mode = 0;
        input_file = fopen(argv[1], "r");
    }

    tcsetpgrp(STDIN, getpid());
    setpgid(0, 0);
    sigset_t set;
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);

    number_of_jobs = 0;
    char command[COMMAND_SIZE + 1];
    
    if(shell_mode)
        print_prompt();

    while(take_string(input_file, command) != EOF){
        int cmdlen = strlen(command);
        trim(command, cmdlen);
        char** sequence;
        int seq_len = tokenize(command, strlen(command), ';', &sequence);
        for(int i = 0; i < seq_len; i++){
            split(sequence[i]);
            check_done_bg();
            run_bg();
            run_fg();
            
        }
        if(shell_mode)
            print_prompt();
        // destroy_tokens(sequence, seq_len);
    }
    printf("\n");
    if(!shell_mode)
        fclose(input_file);
}
