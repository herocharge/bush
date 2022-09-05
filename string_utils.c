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
    for(int i = 0; i < count; i++){
        trim((*out)[i], strlen((*out)[i]));
    }
    return count;
}
void destroy_tokens(char** p, int len){
    for(int i = 0; i < len; i++){
        free(p[i]);
    }
    free(p);
}

// https://stackoverflow.com/questions/779875/what-function-is-to-replace-a-substring-from-a-string-in-c

// You must free the result if result is non-NULL.
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

int is_substr(char* text, char* pattern){
    int len1 = strlen(text);
    int len2 = strlen(pattern);
    for(int i = 0; i < len1 - len2 + 1; i++){
        int ok = 1;
        for(int j = 0; j < len2; j++){
            if(text[i + j] != pattern[j]){
                ok = 0;
                break;
            }
        }
        if(ok) 
            return i+1;
    }
    return 0;
}