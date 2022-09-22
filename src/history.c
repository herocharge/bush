#include "../include/history.h"

int min(int a, int b){
    if(a < b)return a;
    return b;
}

void print_history(char* home){
    char** cmds;
    char path[PATH_MAX+1];
    strcpy(path, home);
    strcat(path, "/.bush_history");
    FILE* histfile = fopen(path, "r");
    int no_of_cmds = 0;
    char allcmd[(NO_OF_CMDS + 1)*(MAX_CMD_LEN)];
    if(histfile != NULL){
        take_string(histfile, allcmd);
        no_of_cmds = tokenize(allcmd, strlen(allcmd), ';', &cmds);
        no_of_cmds--;
        fclose(histfile);
    }
    int start = max(0, no_of_cmds-10);
    int end = min(no_of_cmds, start + 11);
    for(int i = start; i < end; i++){
        printf("%s\n", cmds[i]);
    }
}

void get_history(char* q, int n, char* home){
    char** cmds;
    char path[PATH_MAX+1];
    strcpy(path, home);
    strcat(path, "/.bush_history");
    FILE* histfile = fopen(path, "r");
    int no_of_cmds = 0;
    char allcmd[(NO_OF_CMDS + 1)*(MAX_CMD_LEN)];
    if(histfile != NULL){
        take_string(histfile, allcmd);
        no_of_cmds = tokenize(allcmd, strlen(allcmd), ';', &cmds);
        no_of_cmds--;
        fclose(histfile);
    }
    int start = max(0, no_of_cmds-10);
    int end = min(no_of_cmds, start + 11);
    // for(int i = start; i < end; i++){
    //     printf("%s\n", cmds[i]);
    // }
    strcpy(q, cmds[NO_OF_CMDS - n]);
}

void update_history(char* cmd, char* home){
    if(strlen(cmd) == 0)return;
    // printf("cmd: %s\n", cmd);
    char** cmds;
    char path[PATH_MAX+1];
    strcpy(path, home);
    strcat(path, "/.bush_history");
    FILE* histfile = fopen(path, "r");
    int no_of_cmds = 0;
    char allcmd[(NO_OF_CMDS + 1)*(MAX_CMD_LEN)];
    if(histfile != NULL){
        take_string(histfile, allcmd);
        // printf("%s\n", allcmd);
        no_of_cmds = tokenize(allcmd, strlen(allcmd), ';', &cmds);
        while(!strlen(cmds[no_of_cmds-- - 1]));
        no_of_cmds++;
        fclose(histfile);
    }
    else{
        cmds = malloc(sizeof(char*));
        cmds[0] = malloc(sizeof(char) * MAX_CMD_LEN);
    }
    // printf("%d\n", no_of_cmds);
    // for(int i = 0; i < no_of_cmds; i++){
    //     printf("cmd%d %s\n", i, cmds[i]);
    // }
    if(no_of_cmds && !strcmp(cmd, cmds[no_of_cmds-1])){
        // printf("bruh1\n");
        return;
    }
    if(no_of_cmds >= NO_OF_CMDS){
        for(int i = 0; i < no_of_cmds-1; i++){
            cmds[i] = realloc(cmds[i], sizeof(char) * MAX_CMD_LEN);
            strcpy(cmds[i], cmds[i+1]);
            // printf("bruh2\n");
        }
        cmds[no_of_cmds-1] = realloc(cmds[no_of_cmds-1], sizeof(char) * MAX_CMD_LEN);
    }else{
        no_of_cmds++;
        cmds = realloc(cmds, sizeof(char *) * no_of_cmds);
        cmds[no_of_cmds-1] = malloc(sizeof(char) * MAX_CMD_LEN);
    }
    // printf("%d\n", no_of_cmds);
    strcpy(cmds[no_of_cmds-1], cmd);
    histfile = fopen(path, "w");
    for(int i = 0; i < no_of_cmds; i++){
        // printf("%s\n", cmds[i]);
        fprintf(histfile, "%s;", cmds[i]);
    }
    while(fsync(fileno(histfile)));
    fclose(histfile);
}