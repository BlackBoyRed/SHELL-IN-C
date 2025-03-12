#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "./lib/linenoise.h"

#define PROMPT "👉 "
#define HISTORY_LENGTH 1024
#define MAX_ARGS 1024
#define TOKEN_SEP " \t"
#define PATH_MAX 4096

char CWD[PATH_MAX];

//read / token function
int s_read(char * input, char ** args) {
  int i = 0;
  char * token = strtok(input, TOKEN_SEP);
  while (token != NULL && i < (MAX_ARGS - 1)) {
    args[i++] = token;
    token = strtok(NULL, " \t");
  }
  args[i] = NULL;
  return i;
}

//execute
int s_execute(char * cmd, char ** cmd_args) {
  fprintf(stdout, "Executing `%s`!\n", cmd);

  int status;
  pid_t pid;

  pid = fork();
  if (pid < 0) {
    fprintf(stderr, "Could not execute!\n");
    return -1;
  }

  if (pid == 0) {
    execvp(cmd, cmd_args);
  } else {
    // Parent process would be waiting for the child process
    if (waitpid(pid, & status, 0) != pid) {
      fprintf(stderr, "Could not wait for child process!\n");
      return -1;
    }
  }

  return status;
}

//built-in function

typedef enum Builtin {
  CT,
  WHERE,
  INVALID
}
Builtin;

void builtin_impl_ct(char ** args, size_t n_args);
void builtin_impl_where(char ** args, size_t n_args);

void( * BUILTIN_TABLE[])(char ** args, size_t n_args) = {
  [CT] = builtin_impl_ct,
  [WHERE] = builtin_impl_where,
};

Builtin builtin_code(char * cmd) {
  if (!strncmp(cmd, "ct", 2)) {
    return CT;
  } else if (!strncmp(cmd, "where", 5)) {
    return WHERE;
  } else {
    return INVALID;
  }
}

int is_builtin(char * cmd) {
  return builtin_code(cmd) != INVALID;
}

void s_execute_builtin(char * cmd, char ** args, size_t n_args) {
  BUILTIN_TABLE[builtin_code(cmd)](args, n_args);
}

void refresh_cwd(void) {
  if (getcwd(CWD, sizeof(CWD)) == NULL) {
    fprintf(stderr, "Error: Could not read working dir");
    exit(1);
  }
}

void builtin_impl_ct(char ** args, size_t n_args) {
  char * new_dir = * args;
  if (chdir(new_dir) != 0) {
    fprintf(stderr, "Error: Could not change directory");
    return;
  }
  refresh_cwd();
}

void builtin_impl_where(char ** args, size_t n_args) {
  fprintf(stdout, "%s\n", CWD);
}

int main(void) {
  if (!linenoiseHistorySetMaxLen(HISTORY_LENGTH)) {
    fprintf(stderr, "Could not set linenoise history!");
    exit(1);
  }

  char * line;
  char * args[MAX_ARGS];
  while ((line = linenoise(PROMPT)) != NULL) {

    //read step

    int args_read = s_read(line, args);
    fprintf(stdout, "Read %d args\n", args_read);
    for (int i = 0; i < args_read; i++) {
      fprintf(stdout, "arg[%d] = %s\n", i, args[i]);
    }

    // skip empty lines
    if (args_read == 0) {
      linenoiseFree(line);
      continue;
    }

    //eval + print step
    char * cmd = args[0];
    char ** cmd_args = args;
    if (is_builtin(cmd)) {
      // make sure builtin functions see as the first argument the
      // real first argument, and not the name of the builtin being
      // called
      s_execute_builtin(cmd, (cmd_args + 1), args_read - 1);
    } else {
      s_execute(cmd, cmd_args);

    }

    linenoiseHistoryAdd(line);
    linenoiseFree(line);
  }

  return 0;
}
