#ifndef PINFO_H
#define PINFO_H

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <limits.h>
#include <unistd.h>

void pinfo(int argc, int pid);

#endif