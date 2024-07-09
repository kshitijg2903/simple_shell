#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_SIZE 100
#define MAX_PIPES 5

// Data structure for holding command history
struct cmd_History {
  char cmd[MAX_ARG_SIZE];
  pid_t pid;
  time_t start_time;
  time_t end_time;
  double duration;
};

struct cmd_History history[100];

int history_count = 0;
int pipe_count = 0;
int pipe_arr[MAX_PIPES];

void save_history(char **args, pid_t pid, time_t starttime, time_t endtime) {
  strcpy(history[history_count].cmd, args[0]);
  history[history_count].pid = pid;
  history[history_count].start_time = starttime;
  history[history_count].end_time = endtime;
  history[history_count].duration = difftime(endtime, starttime);
  history_count++;
}

// Function to check the number of pipes
void find_pipes(char **args) {
  for (int i = 0; args[i] != NULL; i++) {
    if (strcmp(args[i], "|") == 0) {
      pipe_count++;
      pipe_arr[pipe_count] = i;
    }
  }
}

int checkForAnd(char **args){
    int n;
    for(n = 0; args[n]!=NULL; n++);
    if(n>0 && strcmp(args[n-1], "&")==0){
        free(args[n-1]);
        args[n-1] = NULL;
        return 1;
    }//else it returns default 0.
    return 0;
}

int create_pipe(int fd_position[2]) {
  int pipe_creation = pipe(fd_position);
  if (pipe_creation == -1) {
    printf("pipe");
    exit(EXIT_FAILURE);
  }
  return pipe_creation;
}

void launch(char **args) {
  pid_t pid;
  int status;
  time_t start_time, end_time;

  start_time = time(NULL);
  pid = fork();

  int bonus = checkForAnd(args);
  if (pid == 0) {
    char cmd_string[MAX_ARG_SIZE * 2] = "";
    for (int i = 0; args[i] != NULL; i++) {
      strcat(cmd_string, args[i]);
      strcat(cmd_string, " ");
    }
    int system_check = system(cmd_string);
    if (system_check == -1) {
      printf("system-call-error");
      exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
  } 
  else if (pid < 0) {
    printf("fork-error");
  } 
  else {
    if(!bonus){
      do {
        waitpid(pid, &status, WUNTRACED);
      } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }else{
      printf("[started %d] %s\n", pid, args[0]); // Optionally print PID and command for background jobs
    }
    end_time = time(NULL);
    save_history(args,pid, start_time, end_time);
    history_count++;
    }
}

int main() {
  size_t len = 0;
  char *input = NULL;
  char *token;
  char *delim = " \n";
  char **argv = NULL;
  int argc;

  while (1) {
    argc = 0;
    printf("SimpleShell~$ ");

    if (getline(&input, &len, stdin) == -1) {
      printf("getline-error");
      exit(EXIT_FAILURE);
    }

    input[strlen(input) - 1] = '\0';

    token = strtok(input, delim);
    while (token) {
      argv = realloc(argv, sizeof(char *) * (argc + 1));
      argv[argc] = strdup(token);
      argc++;
      token = strtok(NULL, delim);
    }

    argv = realloc(argv, sizeof(char *) * (argc + 1));
    argv[argc] = NULL;

    if (argc > 0) {
        if (strcmp(argv[0], "history") == 0) {
        for (int i = 0; i < history_count; i++) {
          printf("%s (pid: %d, start time: %ld, duration: %.2f seconds)\n",
                 history[i].cmd, history[i].pid, history[i].start_time,
                 history[i].duration);
        }
      }
        else if(strcmp(argv[0], "exit") == 0){
            exit(EXIT_FAILURE);
        } 
        
        else {
        launch(argv);
      }
    }

    for (int i = 0; i < argc; i++) {
      free(argv[i]);
    }
    free(input);
    free(argv);
    argv = NULL;
    input = NULL;
    len = 0;
  }

  return 0;
}
