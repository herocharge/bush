#ifndef HISTORY_H
#define HISTORY_H

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "string_utils.h"
#include <unistd.h>

#define MAX_CMD_LEN 10000
#define NO_OF_CMDS 20

void print_history(char* home);
void update_history(char* cmd, char* home);

#endif