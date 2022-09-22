#ifndef LS_H
#define LS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#include "print_table.h"
#include "path_utils.h"

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

#define MAX_NO_OF_FILES 10000

typedef struct Ls_data
{
    char* name;
    char** data;
}Ls_data;


int ls(int argc, char* argv[], char* home);

#endif