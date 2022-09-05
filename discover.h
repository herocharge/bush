#ifndef DISCOVER_H
#define DISCOVER_H

#include "unistd.h"
#include "string_utils.h"
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

#define MAX_PATH_SIZE 10000

void discover(int argc, char** argv);


#endif