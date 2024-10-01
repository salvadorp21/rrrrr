/***************************************************************************//**
  @file         myshell.c
  @author       Salvador Prieto
  @date         09/25/2024
  @brief        myshell - A custom shell implementation with built-in commands.

  This program implements a simple shell that includes basic commands like
  SETSHELLNAME, SETTERMINATOR, and alias management, as well as executing
  standard Unix commands.
*******************************************************************************/

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*
  Global Variables:
*/
char *shellname = "myshell"; // Default shell name
char *terminator = ">";       // Default prompt terminator

#define MAX_ALIASES 10 // Maximum number of allowed aliases

/*
  Alias Structure
*/
struct Alias {
    char *new_name; // Alias name
    char *old_name; // Original command name
};

struct Alias aliases[MAX_ALIASES];
int alias_count = 0;

/*
  Function Declarations for builtin shell commands:
 */
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);
int setshellname(char **args);
int setterminator(char **args);
int newname(char **args);
int listnewnames(char **args);
int savenewnames(char **args);
int readnewnames(char **args);
int lsh_stop(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "setshellname",
  "setterminator",
  "newname",
  "listnewnames",
  "savenewnames",
  "readnewnames",
  "STOP"
};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit,
  &setshellname,
  &setterminator,
  &newname,
  &listnewnames,
  &savenewnames,
  &readnewnames,
  &lsh_stop
};

/**
   @brief Returns the number of built-in commands available in the shell.
   @return The count of built-in commands.
 */
int lsh_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

/**
   @brief Builtin command: change directory.
   @param args List of args. args[0] is "cd". args[1] is the directory to change to.
   @return Always returns 1 to continue executing.
 */
int lsh_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "lsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("lsh");
        }
    }
    return 1;
}

/**
   @brief Builtin command: print help.
   @param args List of args. Not examined.
   @return Always returns 1 to continue executing.
 */
int lsh_help(char **args) {
    printf("myshell - Available commands:\n");
    printf("HELP: Show this help message.\n");
    printf("STOP: Terminate the shell session.\n");
    printf("SETSHELLNAME <name>: Set the shell prompt name.\n");
    printf("SETTERMINATOR <terminator>: Set the prompt terminator.\n");
    printf("NEWNAME <new_name> <old_name>: Create an alias for a command.\n");
    printf("LISTNEWNAMES: List all aliases.\n");
    printf("SAVENEWNAMES <file_name>: Save aliases to a file.\n");
    printf("READNEWNAMES <file_name>: Read aliases from a file.\n");
    printf("<UNIX_command>: Execute any valid UNIX command.\n");
    return 1;
}

/**
   @brief Builtin command: exit the shell.
   @param args List of args. Not examined.
   @return Always returns 0 to terminate execution.
 */
int lsh_exit(char **args) {
    return 0;
}

/**
   @brief Builtin command: terminate the shell session.
   @param args List of args. Not examined.
   @return Always returns 0 to terminate execution.
 */
int lsh_stop(char **args) {
    return 0; // Returning 0 will stop the main loop
}

/**
   @brief Sets the shell name for the prompt.
   @param args List of args. args[1] is the new shell name.
   @return Always returns 1 to continue executing.
 */
int setshellname(char **args) {
    if (args[1] == NULL) {
        shellname = "myshell";
    } else {
        shellname = args[1];
    }
    return 1;
}

/**
   @brief Sets the terminator for the prompt.
   @param args List of args. args[1] is the new terminator.
   @return Always returns 1 to continue executing.
 */
int setterminator(char **args) {
    if (args[1] == NULL) {
        terminator = ">";
    } else {
        terminator = args[1];
    }
    return 1;
}

/**
   @brief Manages alias creation and deletion.
   @param args List of args. args[1] is the new alias, args[2] is the original command.
   @return Always returns 1 to continue executing.
 */
int newname(char **args) {
    // Check for correct argument count
    if (args[1] == NULL) {
        fprintf(stderr, "Error: expected 1 or 2 arguments to \"newname\"\n");
        return 1;
    }

    // Delete alias if only one argument is provided
    if (args[2] == NULL) {
        // Attempt to delete the alias if it exists
        for (int i = 0; i < alias_count; i++) {
            if (strcmp(aliases[i].new_name, args[1]) == 0) {
                // Shift aliases down to remove the alias
                for (int j = i; j < alias_count - 1; j++) {
                    aliases[j] = aliases[j + 1];
                }
                alias_count--;
                return 1;
            }
        }
        fprintf(stderr, "Alias not found: %s\n", args[1]);
    } else {
        // Add or update alias
        if (alias_count < MAX_ALIASES) {
            aliases[alias_count].new_name = strdup(args[1]);
            aliases[alias_count].old_name = strdup(args[2]);
            alias_count++;
        } else {
            fprintf(stderr, "Maximum number of aliases reached.\n");
        }
    }
    return 1;
}

/**
   @brief Lists all defined aliases.
   @param args List of args. Not examined.
   @return Always returns 1 to continue executing.
 */
int listnewnames(char **args) {
    for (int i = 0; i < alias_count; i++) {
        printf("%s -> %s\n", aliases[i].new_name, aliases[i].old_name);
    }
    return 1;
}

/**
   @brief Saves all aliases to a specified file.
   @param args List of args. args[1] is the file name.
   @return Always returns 1 to continue executing.
 */
int savenewnames(char **args) {
    // Check if the correct argument is provided
    if (args[1] == NULL) {
        fprintf(stderr, "Error: argument 1 expected to \"SAVENEWNAMES\"\n");
        return 1;
    }
    
    FILE *file = fopen(args[1], "w");
    if (!file) {
        perror("Error opening file");
        return 1;
    }
    //iterates over all defined aliases and writes each alias pair (new name and original command name) to the specified file.
    for (int i = 0; i < alias_count; i++) {
        fprintf(file, "%s %s\n", aliases[i].new_name, aliases[i].old_name);
    }
    
    fclose(file);
    return 1;
}

/**
   @brief Reads aliases from a specified file.
   @param args List of args. args[1] is the file name.
   @return Always returns 1 to continue executing.
 */
int readnewnames(char **args) {
    // Check if the correct argument is provided
    if (args[1] == NULL) {
        fprintf(stderr, "Error: argument 1 expected to \"READNEWNAMES\"\n");
        return 1;
    }
    
    FILE *file = fopen(args[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }
    // reads pairs of alias and command names from a file and adds them to the shell's alias list until all pairs are read or the maximum alias limit is reached
    char new_name[256], old_name[256]; 
    while (fscanf(file, "%s %s", new_name, old_name) == 2) {
        if (alias_count < MAX_ALIASES) {
            aliases[alias_count].new_name = strdup(new_name);
            aliases[alias_count].old_name = strdup(old_name);
            alias_count++;
        }
    }
    
    fclose(file);
    return 1;
}

/**
  @brief Launch a program and wait for it to terminate.
  @param args Null terminated list of arguments (including program).
  @return Always returns 1 to continue execution.
 */
int lsh_launch(char **args) {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("lsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("lsh");
    } else {
        // Parent process
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

/**
   @brief Execute shell built-in or launch program.
   @param args Null terminated list of arguments.
   @return 1 if the shell should continue running, 0 if it should terminate.
 */
int lsh_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        // An empty command was entered.
        return 1;
    }

    // Check for alias replacement
    for (i = 0; i < alias_count; i++) {
        if (strcmp(args[0], aliases[i].new_name) == 0) {
            args[0] = aliases[i].old_name;
            break;
        }
    }

    // Check for built-in commands
    for (i = 0; i < lsh_num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    // Launch external command
    return lsh_launch(args);
}

/**
   @brief Read a line of input from stdin.
   @return The line from stdin.
 */
char *lsh_read_line(void) {
#define LSH_RL_BUFSIZE 1024
    int bufsize = LSH_RL_BUFSIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();

        if (c == EOF) {
            exit(EXIT_SUCCESS);
        } else if (c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If we have exceeded the buffer, reallocate.
        if (position >= bufsize) {
            bufsize += LSH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
/**
   @brief Split a line into tokens (very naively).
   @param line The line to be split.
   @return Null-terminated array of tokens.
 */
char **lsh_split_line(char *line) {
    int bufsize = LSH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token, **tokens_backup;

    if (!tokens) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += LSH_TOK_BUFSIZE;
            tokens_backup = tokens;
            tokens = realloc(tokens, bufsize * sizeof(char*));
            if (!tokens) {
                free(tokens_backup);
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

/**
   @brief Loop getting input and executing it.
 */
void lsh_loop(void) {
    char *line;
    char **args;
    int status;

    do {
        printf("%s%s ", shellname, terminator); // Use both shellname and terminator
        line = lsh_read_line();
        args = lsh_split_line(line);
        status = lsh_execute(args);

        // free(line);
        // free(args);
    } while (status);
}

/**
   @brief Main entry point.
   @param argc Argument count.
   @param argv Argument vector.
   @return status code.
 */
int main(int argc, char **argv) {
    // Load config files, if any.

    // Run command loop.
    lsh_loop();

    // Perform any shutdown/cleanup.

    return EXIT_SUCCESS;
}
