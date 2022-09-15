#include "discover.h"

char** init_arr(int sz){
    char ** v = (char **)malloc(sizeof(char *) * sz);
    for(int i = 0; i < sz; i++){
        v[i] = (char *)malloc(PATH_MAX);
    }
    return v;
}

char** insert(char** v, int len, char* s, int* cap){
    if(len == (*cap)){
        v = (char **)realloc(v, sizeof(char **) * 2 * len);
        (*cap) *= 2;
     }
    v[len] = malloc(PATH_MAX);
    strcpy(v[len], s);
    // printf("%d %s\n", len, s);
    return v;
}


int is_dir(char* dir){
    struct stat sb;
    if(lstat(dir, &sb) == -1){
        perror(dir);
        return -1;
    }
    return S_ISDIR(sb.st_mode) && !S_ISLNK(sb.st_mode);
}

void dfs(char* curr, int* no_of_files, char*** files, int* cap, int depth){
    if(!is_dir(curr) || depth > DEPTH_MAX)return;
    DIR* d = opendir(curr);
    if(d == NULL){
        perror(curr);
        return;
    }
    struct dirent* file;
    while((file = readdir(d)) != NULL){
        if(!strcmp(file->d_name, ".."))continue;
        if(!strcmp(file->d_name, "."))continue;
        char* file_path = malloc(PATH_MAX);
        strcpy(file_path, curr);
        strcat(file_path, "/");
        strcat(file_path, file->d_name);
        (*files) = insert(*files, *no_of_files, file_path, cap);
        (*no_of_files)++;
        if(file->d_type != DT_LNK)
            dfs(file_path, no_of_files, files, cap, depth + 1);
        free(file_path);
    }
    closedir(d);
}

void discover(int argc, char** argv, char* home){
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
    char* dir = (char *)malloc(PATH_MAX);
    strcpy(dir, ".");
    char* file = NULL;
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '"'){
            file = strdup(argv[i]);
        }
        else if(argv[i][0] != '-'){
            if(argv[i][0] != '/'){
                if(!strcmp(argv[i], ".") || !strcmp(argv[i], "./"))
                    continue;
                strcat(dir, "/");
                if(!strncmp(argv[i], "./", 2)){
                    strcat(dir, &argv[i][2]);
                   
                }
                else{
                    char* ne = expand(argv[i], home);
                    if(strcmp(ne, argv[i]))
                        strcpy(dir,ne);
                    else
                    strcat(dir, ne);
                }
            }
            else 
                strcpy(dir, argv[i]);
        }
    }
    if(file_flag == 1 && dir_flag == 1){
        file_flag = 0;
        dir_flag = 0;
    }
    if(file == NULL){
        // file_flag = 2;
        
    }
    else{
        // remove quotes
        int flen = strlen(file);
        char dummy[flen+1];
        strcpy(dummy, file);
        dummy[flen-1] = 0;
        strcpy(file, &dummy[1]);
        // printf("%s\n", file);
    }
    // printf("%d\n", file_flag);
    // printf("df: %d ff: %d\n", dir_flag, file_flag);
    
    // printf("%s\n", dir);
    char* fulldir = realpath(dir, NULL);
    char** files = init_arr(100);
    int no_of_files = 1;
    int cap = 100;
    files = insert(files, 0, dir, &cap);
    dfs(dir, &no_of_files, &files, &cap, 0);
    for(int i = 0; i < no_of_files; i++){
        if(file == NULL){
            if(!file_flag && is_dir(files[i])){
                printf("%s/\n", files[i]);
            }
            else if(!dir_flag){
                 printf("%s\n", files[i]);
            }
            continue;
        }
        else{
            int ind = 0;
            int len = strlen(file);
            if((ind = is_substr(files[i], file))!=0){
                if(is_dir(files[i]) && file_flag)continue;
                if(!is_dir(files[i]) && dir_flag)continue;
                for(int j = 0; files[i][j] != '\0'; j++){
                    if(ind - 1 == j)
                        printf("%s%s", COLOR_BOLD, KRED);
                    printf("%c", files[i][j]);
                    if(j == ind - 1 + len - 1)
                        printf("%s", COLOR_OFF);
                }
                if(is_dir(files[i]))
                    printf("/");
                printf("\n");
            }
            // printf("%d %s %s\n", ind, files[i], file);
        }
    }
    if(no_of_files < 100)
        no_of_files = 100;
    for(int i = 0; i < no_of_files; i++){
        free(files[i]);
    }
    free(files);
}