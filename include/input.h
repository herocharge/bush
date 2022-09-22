#include <stdio.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "history.h"
#include "path_utils.h"

#define CSI "\e["
int input_fancy(char* cmd, char* home);
