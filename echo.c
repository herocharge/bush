#include "echo.h"

void echo(int argc, char** argv){
    int start = 1;
    if(argc == 1)return;
    if(!strcmp(argv[1], "-n"))start = 2;
    for(int i = start; i < argc; i++){
        printf("%s", argv[i]);
        if(i != argc - 1)printf(" ");
    }
    if(start == 1)printf("\n");
    return;
}