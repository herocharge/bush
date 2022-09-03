#include "print_table.h"

int max(int a, int b){
    if( a < b) return b;
    return a;
}

void print_table(char*** data, int rows, int cols){
    int lens[cols];
    for(int i = 0; i < cols; i++)lens[i] = 0;
    for(int i = 0; i < cols; i++){
        for(int j = 0; j < rows; j++){
            lens[i] = max(lens[i], strlen(data[j][i]));
        }
    }
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            for(int i = 0; i < lens[j] - strlen(data[i][j]); i++)
                printf(" ");
            printf("%s\t", data[i][j]);
        }
        printf("\n");
    }
}