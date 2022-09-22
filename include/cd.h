#ifndef CD_H
#define CD_H

#include <unistd.h>
#include <pwd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "string_utils.h"

#define CWD_SIZE 10000

int cd(int argc, char** argv, char* oldpwd, char* home);
int changedir(int argc, char* argv, char* oldpwd, char* home);

#endif