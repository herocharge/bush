#include "ls.h"

int cmp(const void* a, const void* b){
    Ls_data* x = (Ls_data *)a;
    Ls_data* y = (Ls_data *)b;
    return strcmp(x->name, y->name);
}

int ls(int argc, char* argv[], char* home){
    int list_flag = 0;
    int all_flag = 0;
    int flag;
    while((flag = getopt(argc, argv, "la")) != -1){
        if(flag == (int)'a')all_flag = 1;
        if(flag == (int)'l')list_flag = 1;
    }
    int indices[argc];
    int ind = 0;
    for(int i = 1; i < argc; i++){
        if(argv[i][0] == '-')continue;
        indices[ind] = i;
        ind++;
    }
    for(int i = 0; i < ind; i++){
        int retval;
        // if(!strcmp(argv[indices[i]], "~"))
        retval = actual_ls(argv[indices[i]], list_flag, all_flag);
        if(retval == -1){
            perror("ls");
            return -1;
        }
    }
    return 0;
}
//race conditions galore


char** get_info(struct stat sb, char* file){
    const int no_of_cols = 10;
    const int max_no_size = 20;
    const char * months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    const int max_path_size = 10000;
    char perms[] = "----------";
    if(S_ISREG(sb.st_mode) )perms[0]='-'; // is it a regular file?
    if(S_ISDIR(sb.st_mode) )perms[0]='d'; // directory?
    if(S_ISCHR(sb.st_mode) )perms[0]='c'; // character device?
    if(S_ISBLK(sb.st_mode) )perms[0]='b'; // block device?
    if(S_ISFIFO(sb.st_mode))perms[0]='p'; // FIFO (named pipe)?
    if(S_ISLNK(sb.st_mode) )perms[0]='l'; // symbolic link?  (Not in POSIX.1-1996.)
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

    char** data = (char **)malloc(sizeof(char *) * no_of_cols);
    data[0] = strdup(perms);

    // Number of links
    data[1] = (char *)malloc(sizeof(char) * max_no_size);
    sprintf(data[1], "%d", sb.st_nlink);

    // User name of owner
    struct passwd* unamepswd = getpwuid(sb.st_uid);
    data[2] = strdup(unamepswd->pw_name);

    // owner group
    struct group* grp = getgrgid(sb.st_gid);
    data[3] = strdup(grp->gr_name);

    // Size
    data[4] = (char *)malloc(sizeof(char) * max_no_size);
    sprintf(data[4], "%d", sb.st_size);

    // Last mod time 
    // month
    struct tm* time_of_last_mod = localtime(sb.st_mtim.tv_sec);
    data[5] = strdup(months[time_of_last_mod->tm_mon]);
    // date
    data[6] = (char *)malloc(sizeof(char) * max_no_size);
    sprintf(data[6], "%d", time_of_last_mod->tm_mday);
    // time or year
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

    data[8] = (char *)malloc(sizeof(char) * max_path_size);

    if(perms[0] == 'l'){
        char buf[max_path_size];
        readlink(file, buf, max_path_size);
        data[9] = (char *)malloc(sizeof(char) * max_path_size);
        sprintf(data[8], "%s%s%s", KCYN, file, KWHT);
        sprintf(data[9], "-> %s%s%s", KBLU, buf, KWHT);
    }
    else if(perms[0] == 'd'){
        sprintf(data[8], "%s%s%s", KBLU, file, KWHT);
    }
    else if(perms[0] == 'c'){
        sprintf(data[8], "%s%s%s", KYEL, file, KWHT);
    }
    else{
        data[9] = strdup("");
    }

    return data;
}

int actual_ls(char* file_name, int list_flag, int all_flag){
    struct stat statbuf;
    if(stat(file_name, &statbuf) == -1){
        perror("ls");
        return -1;
    }
    Ls_data d;
    d.name = file_name;
    d.data = get_info(statbuf, d.name);
    int isdir = IS_DIR(statbuf.st_mode);
    Ls_data all[statbuf.st_nlink];
    int len = 1;
    if(!isdir){
        all[0] = d;
    }
    else{
        int ind = 0;
        DIR* dir = opendir(file_name);
        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            all[ind].name = entry->d_name;
            struct stat statbuf;
            if(stat(all[ind].name, &statbuf) == -1){
                perror("ls");
                return -1;
            }
            all[ind].data = get_info(statbuf, all[ind].name);
            ind++;
        }
        len = ind;
        closedir(dir);
        qsort(all, ind, sizeof(Ls_data), cmp);
    }
    char*** dirdata = (char ***) malloc(sizeof(char **) * len);
    if(!list_flag){
        for(int i = 0; i < len; i++){
            if(!all_flag && all[i].name[0] == '.')continue;
            dirdata[i] = all[i].data;
        }
        print_table(dirdata, len, 10);
    }
    else{
        for(int i = 0; i < len; i++){
            if(!all_flag && all[i].name[0] == '.')continue;
            printf("%s\t", all[i].data[8]);
        }
    }
    return 0;
}
