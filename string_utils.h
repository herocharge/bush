#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
int take_string(FILE* input_file, char* s);
void trim(char* text, int len);

int tokenize(char* s, int len, char delim, char*** out);
void destroy_tokens(char** p, int len);
char *str_replace(char *orig, char *rep, char *with);
int is_substr(char* text, char* pattern);

#endif