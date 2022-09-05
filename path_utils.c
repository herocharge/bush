#include "path_utils.h"

char* join(char** paths, int size){
    int len = 1;
    for(int i = 0; i < size; i++){
        len += strlen(paths[i]) + 1;
    }
    char* big = (char *)malloc((sizeof(char)) * (len+1));
    strcpy(big, paths[0]);
    for(int i = 1; i < size; i++){
        strcat(big, "/");
        strcat(big, paths[i]);
    }
    return big;
}

char* parentpath(char* path){
    int len = strlen(path);
    char* p = strdup(path);
    if(p[len-1] == '/'){
        p[len-1] = '\0';
        len--;
    }
    for(int i = len-1;i>=0;i--){
        if(p[i] == '/')break;
        p[i] = 0;
    }
    if(!strlen(p))p[0] = '.';
    return p;
}

char* expand(char* path, char* home){
    char** tokens;
    int len = strlen(path);
    int tokenc = tokenize(path, strlen(path), '/', &tokens);
    char* big = (char *)malloc((strlen(home) + 10 + len) * sizeof(char));
    if(!strcmp(tokens[0], "~")){
            char* p[tokenc];
            p[0] = strdup(home);
            for(int i = 1; i < tokenc; i++)
                p[i] = strdup(tokens[i]);
            char* joined = join(p, tokenc);
            strcpy(big, joined);
    }
    else{
        strcpy(big, path);
    }
    return big;
}