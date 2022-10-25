// imperative programming sucks

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pwd.h>
#include <fcntl.h>
#include <signal.h>

#include "../include/string_utils.h"
#include "../include/echo.h"
#include "../include/cd.h"
#include "../include/jobs.h"
#include "../include/ls.h"
#include "../include/discover.h"
#include "../include/pinfo.h"
#include "../include/input.h"
#include "../include/history.h"
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


void sigchld_handler2(int sigy, siginfo_t *info, void *ucontext){
    // printf("%d %d \n", info->si_value.sival_int, info->si_pid);
    int pid, status;
    while((pid = waitpid(-1, &status, WNOHANG)) > 0); // just to reap zombies
    pid = info->si_pid;
    int jid = pidtojid[pid];
    if(jobs[jid].is_fg && info->si_value.sival_int != SIGCONT){
        // printf("entrer\n");
        tcsetpgrp(STDIN_FILENO, getpgrp());
        tcsetpgrp(STDOUT_FILENO, getpgrp());
    }
    
    // jobs[jid].is_fg = 0;
    // printf("1\n");
    // jobs[jid].pid = pid;
    // jobs[jid].pgid = info->si_gid;
    jobs[jid].exit_code = info->si_status;
    jobs[jid].sig_code = info->si_value.sival_int;
    if(jobs[jid].sig_code != SIGCONT){
        jobs[jid].status_printed = 0;
        int sig = jobs[jid].exit_code;
        if(sig == SIGTTOU || sig == SIGTTIN || sig == SIGSTOP || sig == SIGTSTP)
            jobs[jid].stat = STOPPED;
        else if( sig ==  SIGALRM  ||  sig == SIGHUP   || sig == SIGINT   || sig == SIGIO    || sig == SIGKILL  ||  sig == SIGPIPE || sig == SIGPOLL  || sig == SIGPROF  || sig == SIGPWR   || sig == SIGSTKFLT|| sig == SIGTERM  || sig == SIGUSR1  || sig == SIGUSR2  || sig == SIGVTALRM)
            jobs[jid].stat = KILLED;
        else
            jobs[jid].stat = DONE; 
    }
    else{
        jobs[jid].status_printed = 1;
        jobs[jid].stat = RUNNING; 
    }
    // exit(0);
    // jobs[jid].stat = 10;
    // printf("%d %s %d %d", pid, jobs[jid].cmd, jid, jobs[jid].stat);
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
        printf("\n|____[%s%s%s@%s:%s%s%s (took %.2lfs)%s$ %s]\n", COLOR_BOLD, KMAG, uname, sysname, KBLU, cwd, KYEL,latest_exit_time,KBLU, COLOR_OFF);
        latest_exit_time = 0;
    }
    fflush(stdout);
}

void run(int jobno, int infd, int outfd){
    // printf("Currently running: %s, with pid: %d id fg? %d\n", jobs[jobno].cmd, jobs[jobno].pid, jobs[jobno].is_fg);
    // printf("is_fg: %d\n", jobs[jobno].is_fg);
    char** argvb = NULL;
    int cmdlen = strlen(jobs[jobno].cmd);
    int argc = tokenize(jobs[jobno].cmd, cmdlen, ' ', &argvb);
    argvb = (char **)realloc(argvb, (sizeof(char*))*(argc + 1));
    argvb[argc] = NULL;
    // printf("%d\n", argc);
    
    int inputfd = infd;
    int outputfd = outfd;
    int remove[] = {-1, -1, -1, -1, -1, -1};
    for(int i = 0; i < argc; i++){
        if(strlen(argvb[i]) > 1 && argvb[i][0] == '>' && argvb[i][1] == '>'){
            // printf(">>\n");
            if(strlen(argvb[i]) != 2){
                outputfd = open(&argvb[i][2], O_CREAT | O_WRONLY | O_APPEND , S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if(outputfd == -1){
                    perror(&argvb[i][2]);
                    return;
                }
                remove[4] = i;
            }
            else{
                if(i != argc-1){
                    outputfd = open(argvb[i+1], O_CREAT | O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    if(outputfd == -1){
                        perror(argvb[i+1]);
                        return;
                    }
                    remove[4] = i;
                    remove[5] = i+1;

                }
                else{
                    printf("Bush: No file name provided for redirection\n");
                    return;
                }
            }
        }
        else if(argvb[i][0] == '>'){
            // printf(">\n");
            if(strlen(argvb[i]) != 1){
                outputfd = open(&argvb[i][1], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if(outputfd == -1){
                    perror(&argvb[i][1]);
                    return;
                }
                remove[0] = i;
            }
            else{
                if(i != argc-1){
                    outputfd = open(argvb[i+1], O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                    if(outputfd == -1){
                        perror(argvb[i+1]);
                        return;
                    }
                    printf("%s\n", argvb[i+1]);
                    remove[0] = i;
                    remove[1] = i+1;
                }
                else{
                    printf("Bush: No file name provided for redirection\n");
                    return;
                }
            }
        }
        else if(argvb[i][0] == '<'){
            // printf("<\n");
            if(strlen(argvb[i]) != 1){
                inputfd = open(&argvb[i][1], O_RDONLY);
                if(inputfd == -1){
                    perror(&argvb[i][1]);
                    return;
                }
                remove[2] = i;
            }
            else{
                if(i != argc-1){
                    inputfd = open(argvb[i+1], O_RDONLY);
                    if(inputfd == -1){
                        perror(argvb[i+1]);
                        return;
                    }
                    remove[2] = i;
                    remove[3] = i+1;
                }
                else{
                    printf("Bush: No file name provided for redirection\n");
                    return;
                }
            }
        }
    }
    int nargc = argc;
    for(int i = 0; i < 6; i++){
        // printf("%d ", remove[i]);
        if(remove[i]!=-1)
            nargc--;
    }
    char** argv = (char **)malloc((sizeof(char*))*(nargc + 1));
    int argv_index = 0;
    for(int i = 0; i < argc; i++){
        int ok = 1;
        for(int j = 0; j<6; j++){
            if(remove[j] == i){
                ok = 0;
                break;
            }
        }
        if(!ok){
            // free(argvb[i]);
            continue;
        }
        argv[argv_index++] = (argvb[i]);
    }
    // destroy_tokens(argvb, argc);
    
    argc = nargc;
    argv[argc] = NULL;
    jobs[jobno].input = fdopen(inputfd, "r");
    jobs[jobno].output = fdopen(outputfd, "w");
    // printf("%d\n", fileno(jobs[jobno].output));
    // printf("%d %d %s\n", inputfd, outputfd, argv[0]);
    if(!strlen(argv[0]))return;
    // if(!strcmp(argv[0], "echo")){
        //     echo(argc, argv, jobs[jobno].input, jobs[jobno].output);
        // }
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
        jobs[jobno].stat = DONE;
    } 
    else if(!strcmp(argv[0], "fg")){
        if(argv[1] == NULL){
            printf("fg: Usage: fg <job no>\n");
            jobs[jobno].stat = DONE;
            return;
        }
        int jid = atoi(argv[1]);
        if(jid >= number_of_jobs || jobs[jid].stat == KILLED || jobs[jid].stat == DONE){
            printf("fg: invalid job number\n");
            jobs[jobno].stat = DONE;
            return;
        }
        if(jobs[jid].stat == STOPPED || jobs[jid].stat == RUNNING){
            jobs[jid].is_fg = 1;
            int pid = jobs[jid].pid;
            // printf("before %d %d\n", pid,getpgid(pid));
            tcsetpgrp(STDIN_FILENO, pid);
            tcsetpgrp(STDOUT_FILENO, pid);
            kill(-pid, SIGCONT);
            // tcsetpgrp(STDIN_FILENO, getpgid(pid));
            // tcsetpgrp(STDOUT_FILENO, getpgid(pid));
            int status;
            waitpid(pid, &status, WUNTRACED);
            
            
            // kill(jobs[atoi(argv[1])].pid, SIGCONT);
            // printf("after\n");
            jobs[jobno].stat = DONE;
        }
        jobs[jobno].stat = DONE;
    }
    else if(!strcmp(argv[0], "bg")){
        if(argv[1] == NULL){
            printf("bg: Usage: bg <job no>\n");
            jobs[jobno].stat = DONE;
            return;
        }
        int jid = atoi(argv[1]);
        jobs[jid].is_fg = 0;
        if(jid >= number_of_jobs){
            printf("bg: invalid job number\n");
            jobs[jobno].stat = DONE;
            return;
        }
        int pid = jobs[jid].pid;
        kill(-pid, SIGCONT);
        jobs[jobno].stat = DONE;
    }
    else if(!strcmp(argv[0], "sig")){
        if(argv[1] == NULL || argv[2] == NULL){
            printf("sig: Usage: bg <job no> <sig no>\n");
            printf("Refer to signal(7) for the signal codes\n");
            jobs[jobno].stat = DONE; 
            return;
        }
        int jid = atoi(argv[1]);
        int sno = atoi(argv[2]);
        kill(-jobs[jid].pid, sno);
        perror("sig");
        jobs[jobno].stat = DONE; 
    }
            
    // else if(!strcmp(argv[0], "pwd")){
    //     char pwd[PWD_SIZE];
    //     getcwd(pwd, PWD_SIZE);
    //     printf("%s\n", pwd);
    // }  
    // else if(!strcmp(argv[0], "jobs")){
    //     printf("PID \t PGID \t STAT \t CMD\n");
    //     for(int i =0;i < number_of_jobs;i++){
    //             // if(jobs[i].stat == DONE)continue;
    //             printf("%d \t %d \t %4d \t %s \n", jobs[i].pid, jobs[i].pgid, jobs[i].stat, jobs[i].cmd);
    //     }
    // }
    // else if(!strcmp(argv[0], "fg")){
    //     jobs[atoi(argv[1])].is_fg = 1; 
    //     printf("before %d\n", jobs[atoi(argv[1])].pgid);
    //     kill(jobs[atoi(argv[1])].pgid,SIGCONT);
    //     printf("after\n");
    //     tcsetpgrp(STDIN_FILENO, jobs[atoi(argv[1])].pgid);
    // }
    // else if(!strcmp(argv[0], "ls")){
    //     // for(int i = 1; i < argc; i++){
    //     //     printf("%s ", argv[i]);
    //     // }
    //     // printf("\n");
        
    //     ls(argc, argv, HOME);
    // }
    // else if(!strcmp(argv[0], "discover")){
    //     discover(argc, argv, HOME);
    // }
    // else if(!strcmp(argv[0], "pinfo")){
    //     if(argc == 1){
    //         pinfo(2, getpid());
    //     }
    //     else{
    //         pinfo(2, atoi(argv[1]));
    //     }
    // }
    // else if(!strcmp(argv[0], "exit")){
    //     if(!warning_exit){
    //         int not_dead = 0;
    //         for(int i = 0; i < number_of_jobs; i++){
    //             if(jobs[i].stat != DONE && jobs[i].stat != KILLED && jobs[i].pid != getpid()){
    //                 not_dead = 1;
    //                 break;
    //             }
    //         }
    //         if(not_dead){
    //             warning_exit = 1;
    //             printf("There are still some processes running, You sure you wanna exit?\n");
    //         }
    //         else{
    //             printf("Thank you\n");
    //         for(int i = 0; i < number_of_jobs; i++){
    //             if(jobs[i].stat != DONE && jobs[i].stat != KILLED){
    //                 kill(jobs[i].pid, SIGKILL);
    //             }
    //         }
    //         exit(0);
    //         }
    //     }
    //     else{
    //         printf("Thank you\n");
    //         for(int i = 0; i < number_of_jobs; i++){
    //             if(jobs[i].stat != DONE && jobs[i].stat != KILLED){
    //                 kill(-jobs[i].pid, SIGKILL);
    //             }
    //         }
    //         exit(0);
    //     }
    // }
    // else if(!strcmp(argv[0], "history")){
    //     print_history(HOME);
    // }
    else{
        int backupstdin = dup(infd);
        int backupstdout = dup(outfd);
        pid_t pid = 0;
        if(jobs[jobno].is_fg){
            pid = fork();
            // printf("Forked...\n");
        } 
        
        if(!pid){
            // close(STDIN);
            // close(STDOUT);
            
            if(jobs[jobno].is_fg){
                sigset_t set;
                sigaddset(&set, SIGINT);
                sigaddset(&set, SIGTSTP);
                // sigaddset(&set, SIGSTOP);
                sigprocmask(SIG_UNBLOCK, &set, NULL);
            }
            if(!jobs[jobno].is_fg){
                sigset_t set;
                sigaddset(&set, SIGINT);
                sigaddset(&set, SIGTSTP);
                sigaddset(&set, SIGTTOU);
                sigaddset(&set, SIGTTIN);
                // sigaddset(&set, SIGSTOP);
                sigprocmask(SIG_UNBLOCK, &set, NULL);
            }
            if(jobs[jobno].is_fg){
                setpgid(0, 0);
                // perror("bush");
                // if(inputfd == STDIN)
                    tcsetpgrp(STDIN_FILENO, getpgrp());
                // if(outputfd == STDOUT)
                    tcsetpgrp(STDOUT_FILENO, getpgrp());
                // printf("%d\n", tcgetpgrp(STDIN));
            }
            if(inputfd != STDIN){
                dup2(inputfd, STDIN);
                close(inputfd);
            }
            if(outputfd != STDOUT){
                dup2(outputfd, STDOUT);
                close(outputfd);
            }
            // fsync(0);
            // fsync(1);
                // perror("bush");
            // if(jobs[jobno].is_fg)
            // tcsetpgrp(STDIN, getpid());
            // printf("%d %d %d\n",isatty(0), getpid(), getpgid(getpid()));
            if(!strcmp(argv[0], "echo")){
                echo(argc, argv);
            }
            else if(!strcmp(argv[0], "pwd")){
            char pwd[PWD_SIZE];
            getcwd(pwd, PWD_SIZE);
            printf("%s\n", pwd);
            }  
            else if(!strcmp(argv[0], "jobs")){
                int run_flag = 0, stop_flag = 0;
                if(argc > 1){
                    if(!strcmp(argv[1], "-r")){
                        run_flag = 1;
                    }
                    if(!strcmp(argv[1], "-s")){
                        stop_flag = 1;
                    }
                }
                printf("JID \t PID \t PGID \t STAT \t\t CMD\n");
                for(int i =0;i < number_of_jobs;i++){
                    // if(jobs[i].hide)continue;
                    if(run_flag && jobs[i].stat != RUNNING)continue;
                    if(stop_flag && jobs[i].stat != STOPPED)continue;
                    printf("%d \t %d \t %d \t %s \t %s \n",i, jobs[i].pid, jobs[i].pgid, statstr(jobs[i].stat), jobs[i].cmd);
                }
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
                execvp(argv[0], argv);
                printf("Bush : %s : Command not found\n", argv[0]);
            }
            close(infd);
            dup2(backupstdin, infd);
            close(backupstdin);
            close(outfd);
            dup2(backupstdout, outfd);
            close(backupstdout);
            exit(0);
        }
        else{
            // TODO: Handle signals 
            jobs[jobno].pid = pid;
            jobs[jobno].pgid = pid;
            pidtojid[pid] = jobno;
            int status;
            
            waitpid(pid, &status, WUNTRACED);
            
            // printf("pid: %d\n", pid);
            close(infd);
            dup2(backupstdin, infd);
            close(backupstdin);
            close(outfd);
            dup2(backupstdout, outfd);
            close(backupstdout);
            if(jobs[jobno].is_fg){
                tcsetpgrp(STDIN, getpgid(getpid()));
                tcsetpgrp(STDOUT, getpgid(getpid()));
                // tcsetpgrp(STDIN, pid);
                // printf("%d\n", tcgetpgrp(STDIN));
            }
            // jobs[jobno].is_fg = 0;
            if(jobs[jobno].stat == STOPPED && !jobs[jobno].status_printed ){
                printf("[%d] Stopped %s exit code: %d signal: %s\n", jobno, jobs[jobno].cmd, jobs[jobno].exit_code, strsignal(jobs[jobno].sig_code));
                jobs[jobno].status_printed = 1;
            }
            if(jobs[jobno].stat == KILLED && !jobs[jobno].status_printed ){
                printf("[%d] Killed %s exit code: %d Signal: %s\n", jobno, jobs[jobno].cmd, jobs[jobno].exit_code, strsignal(jobs[jobno].sig_code));
                jobs[jobno].status_printed = 1;
            }
            // printf("debug\n");
            
            
            // printf("debug2 %d\n", status);
            // tcsetpgrp(STDIN, getpid());
        }
    }
    
    // exit(0);
    // destroy_tokens(argv, argc);
}

void pipeline(int jno){
    char** cmds = NULL;
    int  no_of_cmds = tokenize(jobs[jno].cmd, strlen(jobs[jno].cmd), '|', &cmds);
    if(no_of_cmds == 1){
        run(jno, STDIN, STDOUT);
        return;
    }
    jobs[jno].hide = 1;
    int backupstdin = dup(STDIN);
    int backupstdout = dup(STDOUT);
    int lpipe[2]; // i-1 -> i
    // lpipe[0] = 0;
    int rpipe[2]; // i -> i+1
    pipe(rpipe);
    int ind = number_of_jobs;
    jobs[ind] = create_job(jobs[jno].is_fg, RUNNING, cmds[0]);
    jobs[ind].input = stdin;
    jobs[ind].output = stdout;
    
    number_of_jobs++;
    run(ind, STDIN, rpipe[1]);
    // if(!fork()){
    //     dup2(rpipe[1], STDOUT);
    //     close(rpipe[1]);
    //     close(rpipe[0]);
    //     run(ind, 0, 1);
    //     exit(0);
    // }

    // out will become in
    lpipe[0] = rpipe[0];
    lpipe[1] = rpipe[1];
    

    for(int i = 1; i < no_of_cmds-1; i++){
        pipe(rpipe);
        int indy = number_of_jobs;
        jobs[indy] = create_job(jobs[jno].is_fg, RUNNING, cmds[i]);
        jobs[indy].input = stdin;
        jobs[indy].output = stdout;
        jobs[indy].hide = 1;
        number_of_jobs++;
        // if(!fork()){
        //     // make lpipe stdin
        //     dup2(lpipe[0], STDIN);
        //     close(lpipe[0]);
        //     close(lpipe[1]);

        //     // make rpipe stdout           
        //     dup2(rpipe[1], STDOUT);
        //     close(rpipe[1]);
        //     close(rpipe[0]);

        //     run(indy, STDIN, STDOUT);
        //     exit(0);
        // }
        close(lpipe[1]); // otherwise commands block for inputs
        run(indy, lpipe[0], rpipe[1]);
        // we done with inp
        close(lpipe[0]);
        

        // old out is the new inp
        lpipe[0] = rpipe[0];
        lpipe[1] = rpipe[1];
    }
    ind = number_of_jobs;
    jobs[ind] = create_job(jobs[jno].is_fg, RUNNING, cmds[no_of_cmds - 1]);
    jobs[ind].input = stdin;
    jobs[ind].output = stdout;
    jobs[ind].hide = 1;
    number_of_jobs++;
    // if(!fork()){
    //     dup2(lpipe[0], STDIN);
    //     close(lpipe[0]);
    //     close(lpipe[1]);
    //     run(ind, 0, 1);
    //     exit(0);
    // }
    close(lpipe[1]);
    run(ind, lpipe[0], STDOUT);
    close(lpipe[0]);
    
    dup2(backupstdin, STDIN);
    dup2(backupstdout, STDOUT);
    close(backupstdin);
    close(backupstdout);
}

void run_fg(){
    for(int i = 0; i < number_of_jobs; i++){
        if(jobs[i].is_fg && jobs[i].stat == READY){
            // printf("fg: %s\n", jobs[i].cmd);
            jobs[i].stat = RUNNING;
            jobs[i].pid = getpid();
            jobs[i].pgid = getpgid(jobs[i].pid);
            pidtojid[jobs[i].pid] = i;
            // printf("jid: %d\n", pidtojid[jobs[i].pid]);
            time_t start,end;
            time (&start);
            // run(i, 0, 1);
            pipeline(i);
            time (&end);
            latest_exit_time = difftime(end, start); 
            // jobs[i].stat = DONE; // Reached for every command
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
                // run(i, 0, 1); 
                pipeline(i);
                // jobs[i].stat = DONE; // Only reached for builtins
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
        number_of_jobs++;
    }

    // Create the one foreground process
    if(strlen(cmds[no_of_cmds - 1])){
        int ind = number_of_jobs;
        jobs[ind] = create_job(1, READY, cmds[no_of_cmds - 1]);
        jobs[ind].input = stdin;
        jobs[ind].output = stdout;
        jobs[ind].hide = 0;
        number_of_jobs++;
    }
    // printf("%s\n", jobs[ind].cmd);
    // run(cmds[no_of_cmds-1], 0);
    destroy_tokens(cmds, no_of_cmds);
}

void check_done_bg(){
    for(int i = 0; i < number_of_jobs; i++){
        if(jobs[i].stat == DONE && !jobs[i].status_printed && jobs[i].is_fg != 1){
            printf("[%d] Done %s exited ", i, jobs[i].cmd);
            jobs[i].status_printed = 1;
            if(!jobs[i].exit_code)
                printf("normally\n");
            else
                printf("abnormally with exitstatus %d\n", jobs[i].exit_code);
        }
        if(jobs[i].stat == STOPPED && !jobs[i].status_printed ){
            printf("[%d] Stopped %s exit code: %d signal: %s\n", i, jobs[i].cmd, jobs[i].exit_code, strsignal(jobs[i].sig_code));
            jobs[i].status_printed = 1;
        }
        if(jobs[i].stat == KILLED && !jobs[i].status_printed ){
            printf("[%d] Killed %s exit code: %d Signal: %s\n", i, jobs[i].cmd, jobs[i].exit_code, strsignal(jobs[i].sig_code));
            jobs[i].status_printed = 1;
        }
    }
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
    new_action.sa_handler = sigchld_handler2;
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
        // printf("cmd: |%s|\n", command);
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
        // printf("fuck\n");
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
