#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>

#define ROW 100
#define COL 100

typedef struct
{
    int row;
    int col;
    int *data;
} matrix;

typedef struct
{
    int val;
    int loc;
} childExec;

static int m1[ROW][COL]; 
static int m2[ROW][COL]; 

//void multipleProcess(char *fOne, char *fTwo);

void multiply(int *toggle, matrix *mOne, matrix *mTwo, int loc, int row, int col)
{
    childExec result;

    result.val = 0;
    result.loc = loc;

    for (int i = 0; i < mTwo->row; i++)
        result.val += mOne->data[row * mOne->col + i] * mTwo->data[mTwo->col * i + col];

    write(toggle[1], &result, sizeof(result));

    close(toggle[1]); 
}


int main(int argc, char *argv[]) {

    childExec result;

    matrix *mOne = malloc(sizeof(matrix));     // Space is set aside for matrix one
    matrix *mTwo = malloc(sizeof(matrix));     // Space is set aside for matrix two
    matrix *mProduct = malloc(sizeof(matrix)); // Space is set aside for matrix

    /* 
    FILE *matrices = fopen(argv[0], "r");
    
    if(matrices == NULL) {
        printf("Can't load file");
        exit(EXIT_FAILURE);
    } */

    //char const* const fileName1 = argv[1]; /* should check that argc > 1 */
    //FILE* file1 = fopen(fileName1, "r"); /* should check the result */
    char line[256];

    int row1 = 0;
    int col1 = 1;
    int row2 = 0;
    int col2 = 1;
    int mat2 = 0;

    //char * token = strtok(buf, " ");

    while (fgets(line, sizeof(line), stdin) != NULL) {
        
        if(mat2 == 0) {
            row1++;
            if(row1 == 1) {
                for (int i = 0; i < strlen(line); i++) {
                    //printf("line %s \n", line[i]);
                    if (isspace(line[i]) != 0) {
 
                        if (i == 0){
                            col1++;
                        }
                        else{
                            if (isspace(line[i-1]) == 0) {
                            //printf("test %s \n", &line[i]);
                                col1++;
                            }

                            if (i == strlen(line) - 1)
                            {
                                col1--;
                            }
                        }
                    }
                    
                }
                //col1--;
            }
            //printf("%s\n", line);

            if (isspace(line[0])) {
                //printf("blank line \n");
                row1--;
                mat2++;
            }
            else{

            int val = 0, col = 0;
            char *token = strtok(line, " ");

            //printf("%s\n", buf);

            while (token) {
                val = atoi(token);
                m1[row1 - 1][col] = val;
                col++;
                token = strtok(NULL, " ");
            }
            }
            //printf("%s \n", line); 
        }

        else {
            row2++;
            if(row2 == 1) {
                for (int i = 0; i < strlen(line); i++) {
                    //printf("line %s \n", &line[i]);
                    if (isspace(line[i]) != 0) {
                        if (i == 0){
                            col2++;
                        }
                        else{
                            if (isspace(line[i-1]) == 0){
                            //printf("test %s \n", &line[i]);
                                col2++;
                            }

                            if (i == strlen(line) - 1)
                            {
                                col2--;
                            }
                        }
                    }
                    
                }
                //col2--;
            }

            int val = 0, col = 0;
            char *token = strtok(line, " ");

            //printf("%s\n", buf);

            while (token) {
                val = atoi(token);
                m2[row2 - 1][col] = val;
                col++;
                token = strtok(NULL, " ");
            }

        }
    }

    /* mOne->row = row1;
    mOne->col = col1; 
    mOne->data = malloc(sizeof(int) * mOne->row * mOne->col); 

    mTwo->row = row2;
    mTwo->col = col2; 
    mTwo->data = malloc(sizeof(int) * mTwo->row * mTwo->col);  */


    /* printf("m1 row: %d \n", row1);
    printf("m1 col: %d \n", col1);
    printf("m2 row: %d \n", row2);
    printf("m2 col: %d \n", col2); */

    if (col1 != row2) {
        printf("Matrices cannot be multiplied due to size \n");
        exit(EXIT_FAILURE);
    }

    //fclose(file1);
    //file1 = fopen(fileName1, "r");

    mOne->row = row1;
    mOne->col = col1;

    // Allocates memory for the size of the matrix and creates pointer to it
    mOne->data = malloc(sizeof(int) * mOne->row * mOne->col);

    // Used to iterate through the file and store the values
    for (int i = 0; i < mOne->row; i++)
        for (int j = 0; j < mOne->col; j++)
            mOne->data[i * mOne->col + j] = m1[i][j];
            //fscanf(stdin, "%d", &mOne->data[i * mOne->col + j]);

    mTwo->row = row2;
    mTwo->col = col2;

    // Allocates memory for the size of the matrix and creates pointer to it
    mTwo->data = malloc(sizeof(int) * mTwo->row * mTwo->col);

    // Used to iterate through the file and store the values
    for (int i = 0; i < mTwo->row; i++)
        for (int j = 0; j < mTwo->col; j++)
            mTwo->data[i * mTwo->col + j] = m2[i][j];
            //fscanf(stdin, "%d", &mTwo->data[i * mTwo->col + j]);


    //fclose(file1);

    /* printf("matrix 1\n");
    for (int i = 0; i < mOne->row; i++)
    {
        for (int j = 0; j < mOne->col; j++)
            printf("%d\t", mOne->data[i * mOne->col + j]);
        printf("\n");
    }
    printf("\n");

    printf("\nmatrix 2\n");
    for (int i = 0; i < mTwo->row; i++)
    {
        for (int j = 0; j < mTwo->col; j++)
            printf("%d\t", mTwo->data[i * mTwo->col + j]);
        printf("\n");
    }
    printf("\n"); */

    mProduct->data = malloc(sizeof(int) * mOne->row * mTwo->col);

    // This is used to distinguish between reading and writing
    int toggleRW[2];

    // Pipe failed
    if (pipe(toggleRW) == -1)
    {
        printf("Pipe Error\n");
        exit(EXIT_FAILURE);
    }

    int children = 0;

    for (int i = 0; i < mOne->row; i++){
        for (int j = 0; j < mTwo->col; j++){

            pid_t pid = fork();

            if (pid < 0) {
                printf("Forking failure, exiting\n");
                exit(EXIT_FAILURE);
            }

            else if (pid == 0) {
                close(toggleRW[0]); // Close Read
                multiply(toggleRW, mOne, mTwo, children, i, j);

                // Freeing child malloc
                free(mOne->data);
                free(mTwo->data);
                free(mProduct->data);
                free(mOne);
                free(mTwo);
                free(mProduct);
                exit(EXIT_SUCCESS);
            }
            children++;
        }
    }

    close(toggleRW[1]); 

    int finished = 0;

    // Used to load in the values from the child process into the product matrix
    while ((finished = read(toggleRW[0], &result, sizeof(childExec))))
        mProduct->data[result.loc] = result.val;

    // Busy wait, required for proper execution
    for (int i = 0; i < (mOne->row * mTwo->col); i++)
        wait(0);

    
    // This is used to display the resulting matrix
    fprintf(stdout, "\n product matrix\n");
    for (int i = 0; i < mOne->row; i++)
    {
        for (int j = 0; j < mTwo->col; j++)
            fprintf(stdout, "%d\t", mProduct->data[i * mTwo->col + j]);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\n");

    // Freeing parent malloc
    free(mOne->data);
    free(mTwo->data);
    free(mProduct->data);
    free(mOne);
    free(mTwo);
    free(mProduct);

    return 0;
    
}


