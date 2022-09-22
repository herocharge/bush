#include "../include/print_table.h"

int max(int a, int b){
    if( a < b) return b;
    return a;
}

void print_table(char*** data, int rows, int cols, int* just){
    // printf("rows: %d, cols: %d\n", rows, cols);
    int lens[cols];
    for(int i = 0; i < cols; i++)lens[i] = 0;
    for(int i = 0; i < cols; i++){
        for(int j = 0; j < rows; j++){
            lens[i] = max(lens[i], strlen(data[j][i]));
            // printf("%s\n", data[j][i]);
        }
    }
    // printf("scanning done\n");
    for(int i = 0; i < rows; i++){
        // printf("%d \n", i);
        for(int j = 0; j < cols; j++){
            if(just[j])
                for(int k = 0; k < lens[j] - strlen(data[i][j]); k++)
                    printf(" ");
            printf("%s", data[i][j]);
            if(!just[j])
                for(int k = 0; k < lens[j] - strlen(data[i][j]); k++)
                    printf(" ");
            printf(" ");
        }
        printf("\n");
    }
}