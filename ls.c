#include "ls.h"

int cmp(const void* a, const void* b){
    Ls_data* x = (Ls_data *)a;
    Ls_data* y = (Ls_data *)b;
    return strcmp(x->name, y->name);
}


//race conditions galore


char** get_info(struct stat sb, char* file, int* total){
    // printf("getting info of %s\n", file);
    // printf("getting 0\n");
    const int no_of_cols = 10;
    const int max_no_size = 20;
    const char * months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const int max_path_size = 1000;
    char perms[] = "----------";
    if(S_ISREG(sb.st_mode) )perms[0]='-'; // is it a regular file?
    if(S_ISDIR(sb.st_mode) )perms[0]='d'; // directory?
    if(S_ISLNK(sb.st_mode) )perms[0]='l'; // symbolic link?  (Not in POSIX.1-1996.)
    if(S_ISCHR(sb.st_mode) )perms[0]='c'; // character device?
    if(S_ISBLK(sb.st_mode) )perms[0]='b'; // block device?
    if(S_ISFIFO(sb.st_mode))perms[0]='p'; // FIFO (named pipe)?
    if(S_ISSOCK(sb.st_mode))perms[0]='s'; // socket?  (Not in POSIX.1-1996.)
    
    if(sb.st_mode&S_IRUSR)perms[1]='r';     //00400   owner has read permission
    if(sb.st_mode&S_IWUSR)perms[2]='w';     //00200   owner has write permission
    if(sb.st_mode&S_IXUSR)perms[3]='x';     //00100   owner has execute permission
    
    if(sb.st_mode&S_IRGRP)perms[4]='r';     //00040   group has read permission
    if(sb.st_mode&S_IWGRP)perms[5]='w';     //00020   group has write permission
    if(sb.st_mode&S_IXGRP)perms[6]='x';     //00010   group has execute permission
    
    if(sb.st_mode&S_IROTH)perms[7]='r';     //00004   others have read permission
    if(sb.st_mode&S_IWOTH)perms[8]='w';     //00002   others have write permission
    if(sb.st_mode&S_IXOTH)perms[9]='x';     //00001   others have execute permission
    
    (*total) += sb.st_blocks;
    // printf("%d\n", sb.st_blksize);

    char** data = (char **)malloc(sizeof(char *) * no_of_cols);
    // printf("hey\n");
    data[0] = strdup(perms);

    // Number of links
    // printf("getting 1\n");
    data[1] = (char *)malloc(sizeof(char) * max_no_size);
    sprintf(data[1], "%lu", sb.st_nlink);

    // User name of owner
    // printf("getting 2\n");
    struct passwd* unamepswd = getpwuid(sb.st_uid);
    data[2] = strdup(unamepswd->pw_name);

    // owner group
    // printf("getting 3\n");
    struct group* grp = getgrgid(sb.st_gid);
    data[3] = strdup(grp->gr_name);

    // Size
    // printf("getting 4\n");
    data[4] = (char *)malloc(sizeof(char) * max_no_size);
    sprintf(data[4], "%ld", sb.st_size);

    // Last mod time 
    // month
    // printf("getting 5\n");
    time_t tolm = sb.st_mtim.tv_sec;
    struct tm* time_of_last_mod = localtime(&tolm);
    data[5] = strdup(months[time_of_last_mod->tm_mon]);
    // date
    // printf("getting 6\n");
    data[6] = (char *)malloc(sizeof(char) * max_no_size);
    sprintf(data[6], "%d", time_of_last_mod->tm_mday);
    // time or year
    // printf("getting 7\n");
    time_t currtime;
    time(&currtime);
    if(difftime(currtime, sb.st_mtim.tv_sec) > 15618600ll){// 6 months in unix time
        data[7] = (char *)malloc(sizeof(char) * max_no_size);
        sprintf(data[7], "%d", time_of_last_mod->tm_year + 1900);
    }
    else{
        data[7] = (char *)malloc(sizeof(char) * 6);
        sprintf(data[7], "%02d:%02d", time_of_last_mod->tm_hour, time_of_last_mod->tm_min);
    }
    // printf("getting 8\n");
    data[8] = (char *)malloc(sizeof(char) * max_path_size);
    
    // printf("gotten\n");
    if(perms[0] == 'l'){
        char buf[max_path_size];
        readlink(file, buf, max_path_size);
        data[9] = (char *)malloc(sizeof(char) * max_path_size);
        sprintf(data[8], "%s%s%s", KCYN, file, KWHT);
        sprintf(data[9], "-> %s%s%s", KBLU, buf, KWHT);
    }
    else if(perms[0] == 'd'){
        sprintf(data[8], "%s%s%s%s/", COLOR_BOLD, KBLU, file, COLOR_OFF);
        data[9] = strdup("");
    }
    else if(perms[0] == 'c'){
        sprintf(data[8], "%s%s%s%s", COLOR_BOLD, KYEL, file, COLOR_OFF);
        data[9] = strdup("");
    }
    else if(perms[3] == 'x' || perms[6] == 'x' || perms[9] == 'x'){
        sprintf(data[8], "%s%s%s%s", COLOR_BOLD, KGRN, file, COLOR_OFF);
        data[9] = strdup("");
    }
    else{
        free(data[8]);
        data[8] = strdup(file);
        data[9] = strdup("");
    }
    // printf("got info\n");
    return data;
}

int actual_ls(char* file_name, int list_flag, int all_flag, char* home){
    file_name = expand(file_name, home);
    // printf("actual ls called for %s\n", file_name);
    struct stat statbuf;
    if(stat(file_name, &statbuf) == -1){
        perror(file_name);
        return -1;
        // printf("err1\n");
    }
    Ls_data d;
    d.name = file_name;
    int dummy;
    d.data = get_info(statbuf, d.name, &dummy);
    int isdir = S_ISDIR(statbuf.st_mode);
    Ls_data* all = malloc((sizeof(Ls_data)) * MAX_NO_OF_FILES);
    // printf("%d\n", statbuf.st_nlink);
    int len = 1;
    int total = 0;
    if(!isdir){
        all[0] = d;
    }
    else{
        int ind = 0;
        DIR* dir = opendir(file_name);
        if(dir == NULL){
            perror(file_name);
            return -1;
        }
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            // printf("here3\n");
            all[ind].name = strdup(entry->d_name);
            // printf("%d %s\n", ind, entry->d_name);
            struct stat statbuf;
            char* path = (char *)malloc(strlen(file_name) + 1 + strlen(all[ind].name) + 1);
            strcpy(path, file_name);
            strcat(path, "/");
            strcat(path, all[ind].name);
            if(stat(path, &statbuf) == -1){
                perror(path);
                // printf("err2\n");
            }
            // printf("getting infoo..\n");
            all[ind].data = get_info(statbuf, all[ind].name, &total);
            // printf("got infoo\n");
            ind++;
        }
        len = ind;
        closedir(dir);
        qsort(all, ind, sizeof(Ls_data), cmp);
    }
    // printf("%d\n", len);
    char*** dirdata = (char ***) malloc(sizeof(char **) * len);
    if(list_flag){
        printf("total: %d\n", total);
        int ind = 0;
        for(int i = 0; i < len; i++){
            if(!all_flag && all[i].name[0] == '.')continue;
            dirdata[ind] = all[i].data;
            ind++;
            // printf("%p %p\n", dirdata[i], all[i].data);
        }
        // for(int i = 0;  i < len;i++){
        //     printf("%p\n", dirdata[i]);
        // }
        int just[] = {0, 1, 0, 0, 1, 0, 1, 0, 0, 0};
        print_table(dirdata, ind, 10, just);
    }
    else{
        for(int i = 0; i < len; i++){
            if(!all_flag && all[i].name[0] == '.')continue;
            printf("%s\t", all[i].data[8]);
            // printf("%s\n", all[i].data[0]);
        }
        printf("\n");
    }
    return 0;
}

int ls(int argc, char* argv[], char* home){
    // printf("ls called\n");
    int list_flag = 0;
    int all_flag = 0;
    int flag;
    optind = 0;
    while((flag = getopt(argc, argv, "la")) != -1){
        if(flag == (int)'a')all_flag = 1;
        if(flag == (int)'l')list_flag = 1;
    }
    // printf("lf: %d, af: %d\n", list_flag, all_flag);
    int indices[argc];
    int ind = 0;
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-')continue;
        indices[ind] = i;
        ind++;
    }
    for(int i = 0; i < ind; i++){
        if(ind-1)
            printf("%s:\n", argv[indices[i]]);
        actual_ls(argv[indices[i]], list_flag, all_flag, home);
    }
    if(ind == 0){
        // char tmp[10000];
        // getcwd(tmp, 10000);
        // printf("%s\n", tmp);
        actual_ls(".", list_flag, all_flag, home);
    }
    return 0;
}