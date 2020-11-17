#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>

#define ROW 100
#define COL 100

static int mOne[ROW][COL]; 
static int mTrans[ROW][COL]; 

int row1 = 0;
int col1 = 1;


//0 0 1 \n 2 3 4 \n 5 6 7 \n


int main(int argc, char *argv[]) {

    char buf[128];
    /* int mat1[ROW][COL];
    int transposedmat[ROW][COL]; */


    int j = 0;
    //printf("what's tje ja[[s");
    buf[0] = 'c';


    //isspace(buf[0]) == 0
    //strlen(buf) != 0
    while (fgets(buf, 128, stdin) != NULL) {
        //fgets(buf, 128, stdin);
        row1++;
        //printf("test %s\n", buf);
        if(row1 == 1) {
            for (int i = 0; i < strlen(buf); i++) {
                //printf("line %s \n", line[i]);
                if (isspace(buf[i]) != 0) {
 
                    if (i == 0){
                        //col1++;
                    }
                    else{
                        if (isspace(buf[i-1]) == 0) {
                        //printf("test %s \n", &line[i]);
                            col1++;
                            //printf("buf: %c\n", buf[i-1]);

                            

                            //mOne[row1 - 1][col1 - 2] = atoi(&buf[i - 1]);
                        }

                        if (i == strlen(buf) - 1)
                        {
                            col1--;
                        }
                    }
                }     
            }
        }

        /* else {
            j = 0;
            for (int i = 0; i < strlen(buf); i++) {
                //printf("TESSSSSSSSSSSSST %d \n", buf[i]);
                if (isspace(buf[i]) != 0) {
 
                    if (i == 0){
                        
                    }
                    else{
                        if (isspace(buf[i-1]) == 0) {
                            //printf("test %s \n", &line[i]);
                            //col1++;
                            //printf("buf: %c\n", buf[i-1]);
/* 
                            int val = 0, coll = 0;
                            char *token = strtok(buf, " ");

                            while (token) {
                                val = atoi(token);
                                mOne[row1 - 1][coll] = val;
                                coll++;
                                token = strtok(NULL, " ");
                            } */

                           /*  mOne[row1 - 1][j] = atoi(&buf[i - 1]);
                            j++; */
                        /*}


                    }
                }     
            }
        } */

        int val = 0, col = 0;
        char *token = strtok(buf, " ");

        //printf("%s\n", buf);

        while (token) {
            val = atoi(token);
            mOne[row1 - 1][col] = val;
            col++;
            token = strtok(NULL, " ");
        }
        //fgets(buf, 128, stdin);
    }
    
    //printf("row: %d\ncol: %d\n", row1, col1);

    //mOne[row1][col1];

    /* for (int i = 0; i < row1; i++)
    {
        for (int j = 0; j < col1; j++)
            printf("%d\t", mOne[i][j]);
        printf("\n");
    }

    printf("\n"); */

    mTrans[col1][row1];

    //adds values into transposed matrix
    for(int i = 0; i < row1; i++) {
        for (int j = 0; j < col1; j++){
            mTrans[j][i] = mOne[i][j];
        }
    }

    printf("Transposed matrix:\n");

    //prints resulting matrix
    for (int i = 0; i < col1; i++)
    {
        for (int j = 0; j < row1; j++)
            fprintf(stdout, "%d\t", mTrans[i][j]);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\n");

    return 1;
}