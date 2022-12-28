#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function declarations for built-in shell commands: */
int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);

/* List of built-in commands, followed by their corresponding functions. */
char *builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char **) = {
    &sh_cd,
    &sh_help,
    &sh_exit,
};

int sh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

/* Built-in function implementation*/
int sh_cd(char **args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "sh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("sh");
        }
    }
    return 1;
}

int sh_help(char **args)
{
    int i;
    printf("eh shell\n");
    printf("the following commands are built-in:\n");

    for (i = 0; i < sh_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("bye for now\n");
    return 1;
}

int sh_exit(char **args)
{
    return 0;
}

#define SH_RL_BUFSIZE 1024
char *sh_read_line(void)
{
    int bufsize = SH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a charater
        c = getchar();

        // If we hit EOF, replace it with a null character and return.
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize) {
            bufsize += SH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "sh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"
char **sh_split_line(char *line)
{
    int bufsize = SH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;
    
        if (position >= bufsize) {
            bufsize += SH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!token) {
                fprintf(stderr, "sh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

int sh_launch(char **args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("sh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("sh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int sh_execute(char **args)
{
    int i;

    if (args[0] == NULL) {
        return 1;
    }

    for (i = 0; i < sh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return sh_launch(args);
}

void sh_loop(void)
{
    char *line;
    char **args;
    int status;

    do {
        printf(" > ");
        line = sh_read_line();
        args = sh_split_line(line);
        status = sh_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char **argv) 
{
    sh_loop();
    return EXIT_SUCCESS;
}
