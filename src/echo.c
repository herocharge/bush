#include "../include/echo.h"

void echo(int argc, char** argv, FILE* inp_file, FILE* out_file){
    int start = 1;
    if(argc == 1)return;
    if(!strcmp(argv[1], "-n"))start = 2;
    for(int i = start; i < argc; i++){
        fprintf(out_file, "%s", argv[i]);
        if(i != argc - 1)fprintf(out_file, " ");
    }
    if(start == 1)fprintf(out_file, "\n");
    return;
}