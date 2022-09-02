#include "string_utils.h"

int take_string(FILE* input_file, char* s){
    int ind = 0;
    int eof = EOF;
    while(fscanf(input_file, "%c", &s[ind]) != EOF){
        if(s[ind] == '\n'){
            s[ind] = '\0';
            eof = 0;
            break;
        }
        ind++;
    }
    return eof;
}

void trim(char* text, int len){
    int ind = 0;
    char bruh[len+1];
    for(int i = 0; i < len; i++){
        if(text[i] == '\t')text[i] = ' ';
        if(text[i] == ' '){
            if(i == 0) continue;
            else if(text[i-1] == ' ' || text[i-1] == ';' || text[i-1] == '&')continue;
        }
        bruh[ind] = text[i];
        ind++;
    }
    if(ind > 0)
        if(bruh[ind-1] == ' ')
            bruh[ind-1] = 0;
    bruh[ind] = 0;
    strcpy(text, bruh);
}

int tokenize(char* s, int len, char delim, char*** out){
    // According to first hand experience strtok is shite, this aint better but it's on me
    int count = 1;
    for(int i = 0; i < len; i++){
        if(s[i] == delim)
            count++;
    }
    *out = (char **) malloc(sizeof(char *) * count);
    int ind = 0, ind2 = 0;
    for(int i = 0; i < len; i++){
        if(s[i] == delim){
            (*out)[ind2] = (char *) malloc(sizeof(char) * (i - ind + 1));
            (*out)[ind2][i - ind] = 0;
            strncpy((*out)[ind2], &s[ind], i - ind);
            ind = i + 1;
            ind2++;
        }
    }
    (*out)[ind2] = (char *) malloc(sizeof(char) * (len - ind + 1));
    (*out)[ind2][len - ind] = 0;
    strncpy((*out)[ind2], &s[ind], len - ind);
    return count;
}
void destroy_tokens(char** p, int len){
    for(int i = 0; i < len; i++){
        free(p[i]);
    }
    free(p);
}