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
#include "discover.h"
#include "pinfo.h"
#include "input.h"
#include "history.h"
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
double latest_exit_time = 0;
int warning_exit = 0;

void sigchld_handler(int sig, siginfo_t *info, void *ucontext)
{
    pid_t pid;
    int status;
    // printf("%d\n", WEXITSTATUS(status));
    // printf("%d\n", info->si_status);
    // printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
    // printf("sighndlr: %d\n", info->si_code);
    if(jobs[pidtojid[info->si_pid]].is_fg){
        tcsetpgrp(STDIN_FILENO, getpgrp());
        tcsetpgrp(STDOUT_FILENO, getpgrp());
    }
    while((pid = waitpid(-1, &status, WNOHANG)) > 0){
        if(jobs[pidtojid[pid]].is_fg){
            // tcsetpgrp(0, getpgrp());
        }
        // printf("dead : %d\n", pid);
        // printf("status: %d\n", status);
        // printf("sighndlr: %d\n", info->si_code);
        if(WIFEXITED(status)){
            // printf("done\n");
            jobs[pidtojid[pid]].stat = DONE;
            printf("done\n");
            
        }
        if(WIFSTOPPED(status)){
            // printf("stopped\n");
            jobs[pidtojid[pid]].stat = STOPPED;
            jobs[pidtojid[pid]].is_fg = 0;
            printf("[%d] %d\n", pidtojid[pid], pid);
            
        }
        if(info->si_status == SIGTSTP){
            // printf("stopped\n");
            jobs[pidtojid[pid]].stat = STOPPED;
            jobs[pidtojid[pid]].is_fg = 0;
            printf("[%d] %d\n", pidtojid[pid], pid);
            
        }
        if(WIFSIGNALED(status)){
            jobs[pidtojid[pid]].stat = KILLED;
            printf("killed\n");
        }
        // printf("%d\n", jobs[pidtojid[pid]].stat);
        // printf("jid: %d\n", pidtojid[pid]);
        jobs[pidtojid[pid]].status_printed = 0;
        jobs[pidtojid[pid]].exit_code = WEXITSTATUS(status);
        jobs[pidtojid[pid]].sig_code = info->si_status;
        
    }
    if(pid == 0){
        if(info->si_status == SIGTTOU || info->si_status == SIGTSTP){
            jobs[pidtojid[info->si_pid]].stat = STOPPED;
            jobs[pidtojid[info->si_pid]].is_fg = 0;
            printf("[%d] %d\n", pidtojid[info->si_pid], info->si_pid);
            jobs[pidtojid[info->si_pid]].status_printed = 0;
            jobs[pidtojid[info->si_pid]].exit_code = status;
            jobs[pidtojid[info->si_pid]].sig_code = info->si_code;
        }
        jobs[pidtojid[info->si_pid]].exit_code = info->si_code;
        jobs[pidtojid[info->si_pid]].sig_code = info->si_status;
    }
    //     printf("pid:  %d\n", pid);
    //     printf("status: %d\n", info->si_status);
    // printf("sigchld triggered\n");
}

void sigtstp_handler(int sig){
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
    printf("\n ____");
    if(latest_exit_time < 1)
        printf("\n|____[%s%s%s@%s:%s%s$ %s]\n", COLOR_BOLD, KMAG, uname, sysname, KBLU, cwd, COLOR_OFF);
    else{
        printf("\n////[%s%s%s@%s:%s%s%s (took %.2lfs)%s$ %s]\n", COLOR_BOLD, KMAG, uname, sysname, KBLU, cwd, KYEL,latest_exit_time,KBLU, COLOR_OFF);
        latest_exit_time = 0;
    }
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
        if(cd(argc, argv, OLDPWD, HOME) == -1){
            perror(argv[1]);
            chdir(tmp);
        }
        else
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
    else if(!strcmp(argv[0], "fg")){
        jobs[atoi(argv[1])].is_fg = 1; 
        printf("before %d\n", jobs[atoi(argv[1])].pgid);
        kill(jobs[atoi(argv[1])].pgid,SIGCONT);
        printf("after\n");
        tcsetpgrp(STDIN_FILENO, jobs[atoi(argv[1])].pgid);
    }
    else if(!strcmp(argv[0], "ls")){
        // for(int i = 1; i < argc; i++){
        //     printf("%s ", argv[i]);
        // }
        // printf("\n");
        
        ls(argc, argv, HOME);
    }
    else if(!strcmp(argv[0], "discover")){
        discover(argc, argv, HOME);
    }
    else if(!strcmp(argv[0], "pinfo")){
        if(argc == 1){
            pinfo(2, getpid());
        }
        else{
            pinfo(2, atoi(argv[1]));
        }
    }
    else if(!strcmp(argv[0], "exit")){
        if(!warning_exit){
            int not_dead = 0;
            for(int i = 0; i < number_of_jobs; i++){
                if(jobs[i].stat != DONE && jobs[i].stat != KILLED && jobs[i].pid != getpid()){
                    not_dead = 1;
                    break;
                }
            }
            if(not_dead){
                warning_exit = 1;
                printf("There are still some processes running, You sure you wanna exit?\n");
            }
            else{
                printf("Thank you\n");
            for(int i = 0; i < number_of_jobs; i++){
                if(jobs[i].stat != DONE && jobs[i].stat != KILLED){
                    kill(jobs[i].pid, SIGKILL);
                }
            }
            exit(0);
            }
        }
        else{
            printf("Thank you\n");
            for(int i = 0; i < number_of_jobs; i++){
                if(jobs[i].stat != DONE && jobs[i].stat != KILLED){
                    kill(-jobs[i].pid, SIGKILL);
                }
            }
            exit(0);
        }
    }
    else if(!strcmp(argv[0], "history")){
        print_history(HOME);
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
                sigaddset(&set, SIGTSTP);
                // sigaddset(&set, SIGSTOP);
                sigprocmask(SIG_UNBLOCK, &set, NULL);
            }
            if(!jobs[jobno].is_fg)
                setpgid(0, 0);
                // perror("bush");
                // tcsetpgrp(STDIN_FILENO, getpgrp());
                // tcsetpgrp(STDOUT_FILENO, getpgrp());
                // perror("bush");
            // if(jobs[jobno].is_fg)
            // tcsetpgrp(STDIN, getpid());
            // printf("%d %d %d\n",isatty(0), getpid(), getpgid(getpid()));
            execvp(argv[0], argv);
            printf("Bush : %s : Command not found\n", argv[0]);
            kill(getpid(), SIGTERM);
        }
        else{
            // TODO: Handle signals 
            // if(jobs[jobno].is_fg)
            //     tcsetpgrp(STDIN, pid);
            if(jobs[jobno].is_fg){
                jobs[jobno].pid = pid;
                jobs[jobno].pgid = getpgid(pid);
            }
            int status;
            // printf("debug\n");
            
            waitpid(pid, &status, WUNTRACED);
            // printf("debug2 %d\n", status);
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
            time_t start,end;
            time (&start);
            run(i);
            time (&end);
            latest_exit_time = difftime(end, start); 
            jobs[i].stat = DONE; // Reached for every command
            break;
        }
    }   
}

void run_bg(){
    
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
                // printf("Running %s with pid %d \n", jobs[i].cmd, jobs[i].pid);
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
        jobs[ind] = create_job(0, READY, cmds[i]);
        jobs[ind].input = stdin;
        jobs[ind].output = stdout;
        int istart = 0; int iend = 0;
        for(int j = 0; cmds[i][j] != 0; j++){
            if(cmds[i][j] == '<'){
                istart = j+1;
                for(iend = istart; cmds[i][iend] != 0 && cmds[i][iend] != '>'; iend++);
                iend--;
                break;
            }
        }
        // printf("%d %d\n", istart, iend);
        char* input_path = (char *)malloc((iend - istart + 2) * (sizeof(char)));
        strncpy(input_path, &cmds[i][istart], iend - istart+1);
        input_path[iend-istart+1] = 0;
        trim(input_path, strlen(input_path));

        int ostart = 0; int oend = 0;
        for(int j = 0; cmds[i][j] != 0; j++){
            if(cmds[i][j] == '>'){
                ostart = j+1;
                for(oend = ostart; cmds[i][oend] != 0 && cmds[i][oend] != '<'; oend++);
                oend--;
                break;
            }
        }
        int append_flag = 0;
        if(cmds[i][ostart] == '>'){
            append_flag = 1;
            ostart++;
        }
        // printf("%d %d\n", istart, iend);
        char* output_path = (char *)malloc((oend - ostart + 2) * (sizeof(char)));
        strncpy(output_path, &cmds[i][ostart], oend - ostart+1);
        output_path[oend-ostart+1] = 0;
        trim(output_path, strlen(output_path));
        printf("%s\n", (input_path));
        printf("%s\n", (output_path));
        number_of_jobs++;
    }

    // Create the one foreground process
    if(strlen(cmds[no_of_cmds - 1])){
        int ind = number_of_jobs;
        jobs[ind] = create_job(1, READY, cmds[no_of_cmds - 1]);
        jobs[ind].input = stdin;
        jobs[ind].output = stdout;
        number_of_jobs++;
    }
    // printf("%s\n", jobs[ind].cmd);
    // run(cmds[no_of_cmds-1], 0);
    destroy_tokens(cmds, no_of_cmds);
}

void check_done_bg(){
    for(int i = 0; i < number_of_jobs; i++){
        if(jobs[i].stat == DONE && !jobs[i].status_printed){
            printf("[%d] Done %s exited ", i, jobs[i].cmd);
            jobs[i].status_printed = 1;
            if(!jobs[i].exit_code)
                printf("normally\n");
            else
                printf("abnormally with exitstatus %d\n", jobs[i].exit_code);
        }
        if(jobs[i].stat == STOPPED && !jobs[i].status_printed && !jobs[i].is_fg){
            printf("[%d] Stopped %s exit code: %d signal: %s\n", i, jobs[i].cmd, jobs[i].exit_code, strsignal(jobs[i].sig_code));
            jobs[i].status_printed = 1;
        }
        if(jobs[i].stat == KILLED && !jobs[i].status_printed && !jobs[i].is_fg){
            printf("[%d] Killed %s exit code: %d Signal: %s\n", i, jobs[i].cmd, jobs[i].exit_code, strsignal(jobs[i].sig_code));
            jobs[i].status_printed = 1;
        }
    }
}

void makebg(){

}

int main(int argc, char** argv){
    pid_t sess_id = setsid();
    printf(" ________  ___  ___  ________  ___  ___     \n|\\   __  \\|\\  \\|\\  \\|\\   ____\\|\\  \\|\\  \\    \n\\ \\  \\|\\ /\\ \\  \\\\\\  \\ \\  \\___|\\ \\  \\\\\\  \\   \n \\ \\   __  \\ \\  \\\\\\  \\ \\_____  \\ \\   __  \\  \n  \\ \\  \\|\\  \\ \\  \\\\\\  \\|____|\\  \\ \\  \\ \\  \\ \n   \\ \\_______\\ \\_______\\____\\_\\  \\ \\__\\ \\__\\\n    \\|_______|\\|_______|\\_________\\|__|\\|__|\n                       \\|_________|         \n                                            \n                                            \n");
    printf("Welcome to Bush - Bhargav's Useless Shell -v 0.0.1 \n");
    printf("Bush pid: %d\n", getpid());
    getcwd(HOME, HOME_SIZE);
    
    // Input method 
    FILE* input_file = stdin;
    int shell_mode = 1;
    if(argc > 1){
        shell_mode = 0;
        input_file = fopen(argv[1], "r");
    }
    setpgid(0, 0);
    tcsetpgrp(STDIN, getpid());
    
    sigset_t set;
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTSTP);
    // sigaddset(&set, SIGSTOP);
    sigaddset(&set, SIGTTIN);
    sigaddset(&set, SIGTTOU);
    sigprocmask(SIG_SETMASK, &set, NULL);

    struct sigaction new_action, old_action;
    new_action.sa_handler = sigchld_handler;
    new_action.sa_flags = 0 | SA_RESTART | SA_SIGINFO;
    sigaction(SIGCHLD, &new_action, &old_action);

    struct sigaction new_action2, old_action2;
    new_action2.sa_handler = sigtstp_handler;
    new_action2.sa_flags = 0;
    
    sigaction(SIGTSTP, &new_action2, &old_action2);

    number_of_jobs = 0;
    char command[COMMAND_SIZE + 1];
    
    if(shell_mode)
        print_prompt();

    // while(take_string(input_file, command) != EOF){
    while(input_fancy(command, HOME) != EOF){
        int cmdlen = strlen(command);
        trim(command, cmdlen);
        char** sequence;
        int seq_len = tokenize(command, strlen(command), ';', &sequence);
        for(int i = 0; i < seq_len; i++){
            if(strlen(sequence[i])){
                update_history(sequence[i], HOME);
                // printf("seq: %s\n", sequence[i]);
            }
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
    for(int i = 0; i < number_of_jobs; i++){
        if(jobs[i].stat != DONE && jobs[i].stat != KILLED){
            kill(jobs[i].pid, SIGKILL);
        }
    }
    printf("\n");
}
