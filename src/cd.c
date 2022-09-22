#include "../include/cd.h"

int cd(int argc, char** argv, char* oldpwd, char* home){
    char** paths;
    if(argc == 1){
        return changedir(argc, argv[1], oldpwd, home);
    }
    if(!strcmp(argv[1], "/")){
        return chdir("/");
    }
    int len = tokenize(argv[1], strlen(argv[1]), '/', &paths);
    if(!strlen(paths[0]))chdir("/");
    for(int i =0 ; i < len;i++){
        if(strlen(paths[i]))
            {if(changedir(argc, paths[i], oldpwd, home) == -1)return -1;}
    }
    destroy_tokens(paths, len);
    return 0;
}

int changedir(int argc, char* argv, char* oldpwd, char* home){
    char cwd[CWD_SIZE];
    getcwd(cwd, CWD_SIZE);
    if(argc == 1 || !strcmp(argv, "~")){
        chdir(home);
    }
    else if(!strcmp(argv, ".")){

    }
    else if(!strcmp(argv, "..")){
        char pcwd[CWD_SIZE];
        strcpy(pcwd, cwd);
        int len = strlen(cwd);
        int taken_out = 0;
        for(int i = len - 2; i > 0; i--){
            if(pcwd[i] == '/'){
                pcwd[i] = 0;
                taken_out = 1;
                break;
            }
        }
        if(taken_out)
            chdir(pcwd);
        else
            chdir("/");
    }
    else if(!strcmp(argv, "-")){
        printf("%s\n", oldpwd);
        chdir(oldpwd);
    }
    else{
        if(chdir(argv) == -1)return -1;
    }
    return 0;
}