#ifndef DISCOVER_H
#define DISCOVER_H

#include "unistd.h"
#include "string_utils.h"
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>

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

#define MAX_PATH_SIZE 10000
#define DEPTH_MAX 30

void discover(int argc, char** argv);


#endif