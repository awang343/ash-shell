#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define LINE_BUFSIZE 20
#define TOKEN_BUFSIZE 5
#define TOKEN_DELIM " \t\r\n\a"

char *read_ash() {
    unsigned long int size = LINE_BUFSIZE, position = 0;
    char *buffer = malloc (sizeof (char) * size);
    int nextchar;
    
    if (!buffer) {
        fprintf(stderr, "ash: allocation error\n");
        exit(EXIT_FAILURE);
    }
    
    while(1) 
    {
        nextchar = getchar();
        if (nextchar == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = (char) nextchar;
        }
        position++;

        if (position > size) {
            size += LINE_BUFSIZE;
            buffer = realloc (buffer, sizeof (char) * size);
        }
    }
}

char **split_ash(char *line) {
    long unsigned int bufsize = TOKEN_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;
    if (!tokens) {
        fprintf(stderr, "ash: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, TOKEN_DELIM);
    tokens[0] = token;
    position++;

    while (token != NULL) {
        token = strtok(NULL, TOKEN_DELIM);
        tokens[position] = token;

        position++;
        if (position >= bufsize) {
            bufsize += TOKEN_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
        }
    }
    tokens[position] = NULL;

    return tokens;
}

int ash_clear(char **args);
int ash_cd(char **args);
int ash_pwd(char **args);
int ash_exit(char **args);

char *builtin_str[] = {
    "clear",
    "cd",
    "pwd",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &ash_clear,
    &ash_cd,
    &ash_pwd,
    &ash_exit
};

int ash_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int ash_clear(char **args) {
    (void)args;
    printf("\033[H\033[J");
    return 1;
}

int ash_cd(char **args) {
    if (!args[1]) {
        printf("No argument specified\n");
        return 2;
    } else if (chdir(args[1]) != 0) {
        printf("Does not exist\n");
        return 2;
    }

    return 1;
}

int ash_pwd(char **args) {
    printf("%s\n", getcwd(NULL, 0));
    (void)args;
    return 1;
}

int ash_exit(char **args) {
    (void)args;
    printf("Exiting...\n");
    return 0;
}

int ash_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("ash");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("ash");
  } else {
    // Parent process
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  (void)wpid;

  return 1;
}

int execute_ash(char **args)
{
    int i;
    if (!args[0]) {
        return 1;
    }
    for (i = 0; i < ash_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return ash_launch(args);
}


void ash_loop(void) {
    char *line;
    char **args;
    int status = 0;

    do {
        printf("%s> ", "ash");
        line = read_ash();
        args = split_ash(line);
        status = execute_ash(args);
        
        free(line);
        free(args);
    }
    while (status);
}

int main() {
    ash_loop();
    return EXIT_SUCCESS;
}
