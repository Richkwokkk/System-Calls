#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define NV 20        // max number of command tokens
#define NL 100       // input buffer size
char line[NL];       // command input buffer

// Function to display the shell prompt
void prompt(void) {
    fprintf(stdout, "\n msh> ");
    fflush(stdout);
}

int main(int argk, char *argv[], char *envp[]) {
    int frkRtnVal;   // value returned by fork sys call
    int wpid;        // value returned by wait
    char *v[NV];     // array of pointers to command line tokens
    char *sep = " \t\n"; // command line token separators
    int i;           // parse index
    int background;  // flag to check if command should be run in background

    while (1) {
        prompt(); 

        // Read the command line input
        if (fgets(line, NL, stdin) == NULL) {
            // Check for end of file or error
            if (feof(stdin)) {
                fprintf(stderr, "EOF detected, exiting...\n");
                exit(0);
            } else {
                perror("fgets error");
                continue;
            }
        }

        // Ignore comments, empty lines, and null commands
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\000')
            continue;

        // Tokenize the input line
        v[0] = strtok(line, sep);
        for (i = 1; i < NV; i++) {
            v[i] = strtok(NULL, sep);
            if (v[i] == NULL)
                break;
        }
        if (i == 0) continue; // No tokens found
        v[i] = NULL; // Null-terminate the token array

        background = 0; // Reset background flag
        // Check if the last token is '&' to run command in background
        if (i > 1 && strcmp(v[i - 1], "&") == 0) {
            background = 1;
            v[i - 1] = NULL; // Remove '&' from the token array
        }

        // Handle 'cd' command separately
        if (strcmp(v[0], "cd") == 0) {
            if (v[1] == NULL) {
                fprintf(stderr, "cd: missing argument\n");
            } else {
                if (chdir(v[1]) != 0) {
                    perror("cd error");
                }
            }
            continue;
        }

        // Fork a child process to execute the command
        switch (frkRtnVal = fork()) {
        case -1: // Error in fork
            perror("fork error");
            break;

        case 0: // Child process
            execvp(v[0], v); // Execute the command
            perror("execvp error"); // Exec failed
            exit(1); // Exit child process with error

        default:
            if (!background) {
                // Wait for child process to complete if not running in background
                if (waitpid(frkRtnVal, NULL, 0) < 0) {
                    perror("waitpid error");
                }
                printf("%s done\n", v[0]);
            } else {
                // Report that command is running in background
                printf("%s running in background\n", v[0]);
            }

            // Check for background processes that have finished
            while ((wpid = waitpid(-1, NULL, WNOHANG)) > 0) {
                printf("Background process %d done\n", wpid);
            }
            if (wpid < 0 && errno != ECHILD) {
                perror("waitpid error");
            }
            break;
        }
    }
}