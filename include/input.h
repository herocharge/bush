#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "history.h"
#include "path_utils.h"

#define CSI "\e["
#define MAX_NO_FILES 65535
int input_fancy(char* cmd, char* home);
