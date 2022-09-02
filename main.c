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
// #include "jobs.h"

#define COMMAND_SIZE 10000
#define CWD_SIZE 10000
#define UNAME_SIZE 10000
#define SYSNAME_SIZE 10000
#define PWD_SIZE 10000
#define MAX_JOBS 1000
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

void sigchld_handler(int sig)
{
    pid_t pid;
    int status;

    while((pid = waitpid(-1, &status, WNOHANG)) > 0)
        ;
}


// Job jobs[MAX_JOBS];
int number_of_jobs;

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

void run(char* cmd, int is_bg){
    char** argv = NULL;
    int cmdlen = strlen(cmd);
    int argc = tokenize(cmd, cmdlen, ' ', &argv);
    argv = (char **)realloc(argv, (sizeof(char*))*(argc + 1));
    argv[argc] = NULL;
    for(int i = 0; i < argc; i++){
        trim(argv[i], strlen(argv[i]));
    }
    // for(int i = 0; i < argc; i++){
    //     printf("%s\n", argv[i]);
    // }
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
        if(cd(argc, argv, OLDPWD, HOME) == -1)printf("Bush : cd : %s : No such directory\n", argv[1]);
        strcpy(OLDPWD, tmp);
    } 
    else if(!strcmp(argv[0], "pwd")){
        char pwd[PWD_SIZE];
        getcwd(pwd, PWD_SIZE);
        printf("%s\n", pwd);
    }  
    else{
        pid_t pid = fork();
        if(!pid){
            // printf("pid: %d\n, %s\n", getppid(), argv[0]);
            // printf("child pid: %d\n", getpid());
            execvp(argv[0], argv);
            printf("Bush : %s : Command not found\n", argv[0]);
            kill(getpid(), SIGTERM);
        }
        else{
            // printf("Pid %d\n", pid);
            int status;
            waitpid(pid, &status, 0);
            // printf("Exited with status - %d\n", status);
            if(is_bg){
                // printf("killing parent pid : %d\n", getpid());
                kill(getpid(), SIGKILL);
            }
        }
    }
    destroy_tokens(argv, argc);
}

void split(char* seq){
    char** cmds;
    int no_of_cmds = tokenize(seq, strlen(seq), '&', &cmds);
    for(int i = 0; i < no_of_cmds; i++){
        trim(cmds[i], strlen(cmds[i]));
    }
    
    // signal(SIGCHLD, sigchld_handler);
    pid_t pids[no_of_cmds];
    for(int i = 0; i < no_of_cmds - 1; i++){
        pid_t pid = fork();
        pids[i] = pid;
        if(!pid){
            
            // Create child process
            setpgid(0, 0);
            struct sigaction new_action, old_action;
            new_action.sa_handler = sigchld_handler;
            sigaction(SIGCHLD, &new_action, &old_action);
            run(cmds[i], 1);
            kill(getpid(), SIGKILL);
        }
        else{
            printf("+[%d] pid : %d\n", i+1, pid);
        }
    }
    
    run(cmds[no_of_cmds-1], 0);
    destroy_tokens(cmds, no_of_cmds);
}

int main(int argc, char** argv){
    pid_t sess_id = setsid();
    printf("Welcome to Bush -v 0.0.1 \n");
    getcwd(HOME, HOME_SIZE);
    
    // Input method 
    FILE* input_file = stdin;
    int shell_mode = 1;
    if(argc > 1){
        shell_mode = 0;
        input_file = fopen(argv[1], "r");
    }

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
        }
        if(shell_mode)
            print_prompt();
        destroy_tokens(sequence, seq_len);
    }
    printf("\n");
    if(!shell_mode)fclose(input_file);
}
