#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <math.h>
#include <setjmp.h>


#include <stddef.h>


/* typedef struct
  {
    void *ss_sp;
    int ss_flags;
    size_t ss_size;
  } stack_t; 
  
  /* test
  XOPEN_SOURCE >= 500 ||
           _XOPEN_SOURCE && _XOPEN_SOURCE_EXTENDED
           ||  Since glibc 2.12:  _POSIX_C_SOURCE >= 200809L
  
  */

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


#define STACK_SZ 16777216
#define ROW 100
#define COL 100

#define mctx_save(mctx) \
setjmp((mctx)->jb)

#define mctx_restore(mctx) \
longjmp((mctx)->jb, 1)

#define mctx_switch(mctx_old,mctx_new) \
if (setjmp((mctx_old)->jb) == 0 ) \
longjmp((mctx_new)->jb, 1)

typedef struct mctx_st {
    jmp_buf jb;
} mctx_t;

static jmp_buf end;
static int n_thr = 0, max_thr = 0;
static mctx_t thread[ROW * COL];
static char *stack[STACK_SZ];

static mctx_t mctx_caller;
static sig_atomic_t mctx_called;

static mctx_t *mctx_creat;
static void (*mctx_creat_func)(int);
static int mctx_creat_arg1;

static sigset_t mctx_creat_sigs;

static int mOne[ROW][COL]; 
static int mTwo[ROW][COL]; 
static int mProduct[ROW][COL];

int row1 = 0;
int col1 = 1;
int row2 = 0;
int col2 = 1;

void mctx_create_boot() {
    void (*mctx_start_func)(int);
    int mctx_start_arg1;

    sigprocmask(SIG_SETMASK, &mctx_creat_sigs, NULL);
  
    mctx_start_func = mctx_creat_func;
    mctx_start_arg1 = mctx_creat_arg1;
  
    mctx_switch(mctx_creat, &mctx_caller);
  
    mctx_start_func(mctx_start_arg1);
}

void mctx_create_trampoline(int sig) {
    if (mctx_save(mctx_creat) == 0) {
        mctx_called = 1;
        return;
    }
  
    mctx_create_boot();
}

void mctx_create (mctx_t *mctx, void (*sf_addr) (int),  
    int sf_arg1, void *sk_addr, size_t sk_size) {
    struct sigaction sa;
    struct sigaction osa;
    //struct stack_t ss;
    //struct stack_t oss;
    sigset_t osigs;
    sigset_t sigs;

    stack_t ss;
    stack_t oss;
  
    //printf("testing again\n");

    sigemptyset(&sigs);
    sigaddset(&sigs, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigs, &osigs);
  
    memset((void *) &sa, 0, sizeof(struct sigaction));
    sa.sa_handler = mctx_create_trampoline;
    sa.sa_flags = SA_ONSTACK;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, &osa);
    

    
    ss.ss_sp = sk_addr;
    ss.ss_size = sk_size;
    ss.ss_flags = 0;
    sigaltstack(&ss, &oss);
  
    mctx_creat = mctx;
    mctx_creat_func = sf_addr;
    mctx_creat_arg1 = sf_arg1;
    mctx_creat_sigs = osigs;
    mctx_called = 0;
    kill(getpid(), SIGUSR1);
    sigfillset(&sigs);
    sigdelset(&sigs, SIGUSR1);
    while(!mctx_called)
        sigsuspend(&sigs);
  
    sigaltstack(NULL, &ss);
    ss.ss_flags = SS_DISABLE;
    sigaltstack(&ss, NULL);
    if (!(oss.ss_flags & SS_DISABLE))
        sigaltstack(&oss, NULL);
    sigaction(SIGUSR1, &osa, NULL);
    sigprocmask(SIG_SETMASK, &osigs, NULL);
  
    mctx_switch(&mctx_caller, mctx);
  
    return;
}


void my_thr_create(void (*func) (int), int thr_id){

   // printf("test3\n");
    int size = STACK_SZ / 2024;
    stack[thr_id] = (char *) malloc(STACK_SZ / 262144);

    mctx_create(&thread[thr_id], func, thr_id, 
                (void *) stack[thr_id] + size, size);

    //printf("test45\n");
    return;
}


//void multipleProcess(char *fOne, char *fTwo);

void multiply(int loc)
{
    //printf("test10\n");
    //childExec result;
    //int col2 = mTwo->col;
        //printf("test11\n");

    int row = (loc / col2); 
    //printf("row: %d\n", row);

    int col = (loc % col2);
    //printf("col: %d\n", col);

    int val = 0;

    //printf("test4\n");
    for (int i = 0; i < col1; i++) {
        //printf("mOne: %d\n", mOne[row][i]);
        //printf("mTwo: %d\n", mTwo[i][col]);
        val += mOne[row][i] * mTwo[i][col];
    }
    //printf("test5\n");
    mProduct[row][col] = val;
    //printf("test6\n");
    n_thr++;
    //printf("test7\n");
    if (n_thr < max_thr) {
        mctx_switch(&thread[loc], &thread[loc + 1]);
    }
    //printf("test8\n");
    longjmp(end, 1); 
    //printf("test9\n");
    //write(toggle[1], &result, sizeof(result));

    //close(toggle[1]); 
}


int main(int argc, char *argv[]) {

    //childExec result;

    //matrix *mOne = malloc(sizeof(matrix));       // Space is set aside for matrix one
    //matrix *mTwo = malloc(sizeof(matrix));       // Space is set aside for matrix two
    //matrix *mProduct = malloc(sizeof(matrix)); // Space is set aside for matrix

    /* 
    FILE *matrices = fopen(argv[0], "r");
    
    if(matrices == NULL) {
        printf("Can't load file");
        exit(EXIT_FAILURE);
    } */

    //char const* const fileName1 = argv[1]; /* should check that argc > 1 */
    //FILE* file1 = fopen(fileName1, "r"); /* should check the result */
    char line[256];

    
    int m2 = 0;

    while (fgets(line, sizeof(line), stdin) != NULL) {
        
        if(m2 == 0) {
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
                m2++;
            }
            else{

            int val = 0, col = 0;
            char *token = strtok(line, " ");

            //printf("%s\n", buf);

            while (token) {
                val = atoi(token);
                mOne[row1 - 1][col] = val;
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
                mTwo[row2 - 1][col] = val;
                col++;
                token = strtok(NULL, " ");
            }

        }
    }

    /* printf("m1 row: %d \n", row1);
    printf("m1 col: %d \n", col1);
    printf("m2 row: %d \n", row2);
    printf("m2 col: %d \n", col2); */

    if (col1 != row2) {
        printf("Matrices cannot be multiplied due to size \n");
        exit(EXIT_FAILURE);
    }

   /*  fclose(file1);
    file1 = fopen(fileName1, "r"); */

    // mOne->row = row1;
    // mOne->col = col1;

    //mOne[row1][col1];
    // Allocates memory for the size of the matrix and creates pointer to it
    //mOne->data = malloc(sizeof(int) * mOne->row * mOne->col);

    // Used to iterate through the file and store the values
    /* for (int i = 0; i < row1; i++)
        for (int j = 0; j < col1; j++)
            fscanf(file1, "%d", &mOne[i][j]); */

    // mTwo->row = row2;
    // mTwo->col = col2;

    //mTwo[row2][col2];

    // Allocates memory for the size of the matrix and creates pointer to it
    //mTwo->data = malloc(sizeof(int) * mTwo->row * mTwo->col);

    // Used to iterate through the file and store the values
    /* for (int i = 0; i < row2; i++)
        for (int j = 0; j < col2; j++)
            fscanf(file1, "%d", &mTwo[i][j]); */


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

    //mProduct->data = malloc(sizeof(int) * mOne->row * mTwo->col);
    mProduct[row1][col2];


    int children = 0;
    //printf("testing thread create\n");


    for (int i = 0; i < row1; i++){
        for (int j = 0; j < col2; j++){

            int thr_id = ((col2 * i) + j);
            
            //printf("test5\n");

            my_thr_create(multiply, thr_id);
            
            
            //printf("test6\n");

            
            /* pid_t pid = fork();

            if (pid < 0) {
                printf("Forking failure, exiting\n");
                exit(EXIT_FAILURE);
            }

            else if (pid == 0) {
                close(toggleRW[0]); // Close Read

                int thr_id = ((col2 * i) + j);
                my_thr_create(multiply, thr_id);

                multiply(toggleRW, mOne, mTwo, children, i, j); */

                // Freeing child malloc
/*              free(mOne->data);
                free(mTwo->data);
                free(mProduct->data);
                free(mOne);
                free(mTwo);
                free(mProduct); */
                /* exit(EXIT_SUCCESS);
            }
            children++;
        */
        }
    }
    //printf("test1\n");

    max_thr = row1 * col2;


    if (setjmp(end) == 0) {
        mctx_switch(&mctx_caller, &thread[0]);
    }
    //printf("test\n");
    //close(toggleRW[1]); 

    //int finished = 0;

    // Used to load in the values from the child process into the product matrix
    /* while ((finished = read(toggleRW[0], &result, sizeof(childExec))))
        mProduct->data[result.loc] = result.val;
 */
    // Busy wait, required for proper execution
    /* for (int i = 0; i < (mOne->row * mTwo->col); i++)
        wait(0); 
 */

    /* fprintf(stdout, "\n matrix1\n");
    for (int i = 0; i < row1; i++)
    {
        for (int j = 0; j < col1; j++)
            fprintf(stdout, "%d\t", mOne[i][j]);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\n");

        fprintf(stdout, "\n matrix2\n");
    for (int i = 0; i < row2; i++)
    {
        for (int j = 0; j < col2; j++)
            fprintf(stdout, "%d\t", mTwo[i][j]);
        fprintf(stdout, "\n");
    }
    fprintf(stdout, "\n"); */

    
    // This is used to display the resulting matrix
    fprintf(stdout, "\n product matrix\n");
    for (int i = 0; i < row1; i++)
    {
        for (int j = 0; j < col2; j++)
            fprintf(stdout, "%d\t", mProduct[i][j]);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\n");

    // Freeing parent malloc
    /* free(mOne->data);
    free(mTwo->data);
    free(mProduct->data);
    free(mOne);
    free(mTwo);
    free(mProduct); */

    return 0;
    
}
