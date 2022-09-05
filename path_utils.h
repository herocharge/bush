#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include <string.h>
#include "string_utils.h"
#include <unistd.h>

char* expand(char* path, char* home);
char* join(char** paths, int size);
char* parent(char* path);

#endif