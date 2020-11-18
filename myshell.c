/*
Chase Maivald
U18719879
CS410 Assignment 2
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <readline/readline.h>

#define my_pipe(x,y) _my_pipe(x,y,-1)

// pid of parent that we may kill if necessary
pid_t parent_fork = -1;

/// returns size of null-terminated array 
size_t my_sizeof(char* arr[]) {
   int i;
   for (i=0; arr[i] != NULL; i++);
   return i;
}

void error_msg(char *path) {
   switch(errno) {
      case EACCES: {
         printf("myshell: %s: permission denied in search\n", path);
         break;
      }
      case EISDIR: {
         printf("myshell: %s is a directory, specify a file path instead\n", path);
         break;
      }
      default: {
         printf("myshell: %s: an error occurred opening this file\n", path);
         break;
      }
   }
}

// exec wrapper, handles exit statuses inside child process
void _exec(char *args[]) {
   int status;
   if (execvp(args[0], args) < 0) {
      switch (errno) {
         case ENOENT: {
                         printf("%s: cmd not found, see README for existing cmds\n", args[0]);
                         break;
                      }
         case EACCES: {
                         printf("%s: permission denied, cannot execute this cmd\n", args[0]);
                         break;
                      }
         default: {
                     printf("%s: unknown error %d\n", args[0], errno);
                     break;
                  }
      }
      exit(0);
   }
}

// handles < and > operators for stdin/stdout, used after fork
int redirection(char *args[]) {
   int i;
   int redirect_flag = 0;
   for (i=0; args[i] != NULL; i++) {
      int size = strlen(args[i]);
      if (args[i][size-1] == '>' || args[i][size-1] == '<') {
         if (args[i+1] == NULL) {
            printf("myshell: error: no filename specified after '%c'\n", args[i][size-1]); // grab last cmd char before null byte
            exit(-1); // largest exit code for purpose of unknown error truncation, similar to abort
         }

         // copy new fd onto the appropriate fd as 0,1,2
         if (args[i][size-1] == '>') {
            char *path = args[i+1];
            // path, open for write/append/creation
            int fd = open(path, O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP ); // , permission bits for owner/group owner of file                                                                                
            if (fd < 0) { // error has occured
               error_msg(path);
               exit(-1);
            } 

            if (size == 1) 
               dup2(fd,1);
            
            else 
              switch (args[i][size-2]) {
                 case '&': 
                    dup2(fd, 1);
                 case '2': 
                    dup2(fd, 2);  
                    break;
                 case '1':
                    dup2(fd, 1);
                    break;
                 default:
                    printf("myshell: invalid specifier: %s\n", args[i]);
                    exit(-1);
              }
         }
         else if (args[i][size-1] == '<') {
            char *path = args[i+1];
            int fd = open(path, O_RDONLY); // open for read 

            if (fd < 0) {
               error_msg(path);
               exit(-1);
            } 
            dup2(fd, 0);
         }
      // 0xFFFFFFFF on x86, 0xFFFFFFFFFFFFFFFF on 64 bit, invalid address > than anything physically possible
      void* impossibly_large = (void*)-1;
      redirect_flag = 1;
      args[i] = impossibly_large;
      args[i+1] = impossibly_large;
      i++;
      }
   }
   return redirect_flag;
}

void collapse_shell(char *new_args[], char *old_args[]) {
   int i;
   int newarg_index=0;
   void* impossibly_large = (void*) -1;
   for (i=0; old_args[i] != NULL; i++) 
      if (old_args[i] != impossibly_large)
        new_args[newarg_index++] = old_args[i];
   new_args[newarg_index] = NULL; // place null EOF 
}

void _my_pipe(char* args1[], char* args2[], int fd_read) {
   int i, multiple_pipes_flag = 0;
   int fd[2];
   char* args3[my_sizeof(args1)];

   if (args2[0] != NULL) {
      pipe(fd);
      multiple_pipes_flag = 1;
      
      for(i=0; args2[i] != NULL; i++)
         if (strcmp(args2[i], "|") == 0) {
            int j;
            int args3_index = 0;
            for (j=i+1; args2[j] != NULL; j++)
               args3[args3_index++] = args2[j];
            args3[args3_index] = NULL;
            args2[i] = NULL;
            break;
      }
      args3[0] = NULL; // no '|'s in cmd string
   }

   pid_t pid, wpid;
   pid = fork();
   int status;

   if (pid < 0 ) {
      printf("myshell: a piping error occurred\n");
      return;
   }
   else if (pid == 0) { // child time!
      if (multiple_pipes_flag) {
         close(fd[0]);
         dup2(fd[1],1); // redirect stdout to pipe
      }
      if (fd_read > 0)
         dup2(fd_read, 0); // iteration 2

      _exec(args1);
   }
   else { // waiting parent
      parent_fork = pid;

      if (multiple_pipes_flag)
         close(fd[1]);

      wpid = waitpid(pid, &status, 0);
      parent_fork = -1;
      
      if (multiple_pipes_flag)
         _my_pipe(args2, args3, fd[0]);
   }
}

// fork child, then call _exec 
void do_cmd(char* args[], int bg) {
   pid_t cpid, wpid;
   int status;
   cpid = fork();

   if (cpid < 0) {
      printf("myshell: error: pipe can't communicate from parent-child process\n");
      return;
   }
   else if (cpid == 0) { // child
      if (redirection(args)) {
         char *new_args[my_sizeof(args)];
         collapse_shell(new_args, args); // make pipe receive EOF
         _exec(new_args);
      }
      else 
         _exec(args);
   }
   else { // parent
      if (!bg) {
         parent_fork = cpid;
         wpid = waitpid(cpid, &status, 0);
         parent_fork = -1;
         fflush(stdout); // allows "clear"?
      }
   }
}

void read_cmd(char *command) {
   int i;

   if (command == NULL)
      return;

   if (command[0] == '\0')
      return;

   // split cmd by any ; 
   for (i=0; command[i] != '\0'; i++) {
      if (command[i] == ';') {
         read_cmd(command+i+1);
         command[i] = '\0';
         break;  
      }
   }

   int pipe_flag = 0, redirect_flag = 0, ampersand_flag = 0;
   for (i=0; command[i] != '\0'; i++){
      switch (command[i]) {
         case '|': {
            pipe_flag = 1;
            break;
         }
         case '>':
         case '<': {
            redirect_flag = 1;
            break;
         }
         case '&': {
            if ( command[i+1] == '>' )
               break;

            if ( command[i+1] != '\0') {
               printf("myshell: syntax error after '&'\n");
               return;
            }
            ampersand_flag = 1;
            command[i] = '\0';
            i--;
            break;
         }
      }
   }

   if ((pipe_flag && redirect_flag)) {
      printf("myshell: error: '|' operator incompatible with '>' or '<'\n");
      return;
   }
   if ((pipe_flag && ampersand_flag)) {
      printf("myshell: error: '|' operator incompatible with '&'\n");
      return;
   }

   char *args[50]; // 50 cmd args max
   int argindex;
   args[0] = strtok(command, " ");
   for (argindex=1; (args[argindex] = strtok(NULL, " ")) != NULL; argindex++); // jmp over whitespaces

   if (strcmp(args[0], "exit") == 0) {
      exit(0); // ready for successful termination
   }

   if (pipe_flag) {
      // finds pipe operator, then passes it to my_pipe 
      for (i=0; args[i] != NULL; i++) {
         if (strcmp(args[i], "|") == 0) {
            args[i] = NULL;

            char* args2[my_sizeof(args)];
            int j;
            int args2_index = 0;
            for (j=i+1; args[j] != NULL; j++)
               args2[args2_index++] = args[j];

            args2[args2_index] = NULL;
            my_pipe(args, args2);
         }
      }
   }
   else {
      do_cmd(args,ampersand_flag);
   }
}

static void handle_sigchld(int sig, siginfo_t *siginfo, void *context) {
   int status;
   while(waitpid(-1,&status,WNOHANG) > 0) {}
}

static void handle_sigint(int sig, siginfo_t *siginfo, void *context) {
   if (parent_fork > 0)
      kill(parent_fork, SIGINT); // allows CTRL+Z // CTRL+C
   if(isatty(0)) fprintf(stdout, "\n");
}

int main() {
   struct sigaction sigchild_action, sigint_action; // data to examine/change signal action(s)
   memset(&sigchild_action,'\0', sizeof(sigchild_action));
   sigchild_action.sa_sigaction = handle_sigchld;

   memset(&sigint_action, '\0', sizeof(sigint_action));
   sigint_action.sa_sigaction = handle_sigint;

   if (sigaction(SIGCHLD, &sigchild_action, NULL) < 0) {
      printf("sigaction error\n");
      exit(-1);
   }

   if (sigaction(SIGINT, &sigint_action, NULL) < 0) {
      printf("sigaction error\n");
      exit(-1);
   }

   char *line;
   ssize_t linebuf = 0;

   while (1) {
      line = readline("myshell> ");
      /*
      if(getline(&line, &linebuf, stdin) == -1){
         if(isatty(0)) fprintf(stdout, "\n");
         break;
      }
      */
      if(line[0] == '\n') { continue; }
      read_cmd(line);
   }
}