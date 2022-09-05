#include "discover.h"

void discover(int argc, char** argv){
    optind = 0;
    int flag;
    int dir_flag = 0 , file_flag = 0;
    while((flag = getopt(argc, argv, "df")) != -1){
        if(flag == (int)'d')dir_flag = 1;
        if(flag == (int)'f')file_flag = 1;
    }
    if(argc == 1){
        dir_flag = 1;
        file_flag = 1;
    }
    char* dir = (char *)malloc(MAX_PATH_SIZE);
    strcpy(dir, "./");
    char* file = NULL;
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '"'){
            file = strdup(argv[i]);
        }
        else if(argv[i][0] != '-'){
            strcat(dir, argv[i]);
        }
    }
    if(file == NULL)
        file_flag = 1;
    char* fulldir;
    

}