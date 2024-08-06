#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#define NV 20
#define NL 100

char line[NL];

void prompt(void) {
    if (fprintf(stdout, "\n msh> ") < 0) {
        perror("fprintf");
        exit(EXIT_FAILURE);
    }
    if (fflush(stdout) != 0) {
        perror("fflush");
        exit(EXIT_FAILURE);
    }
}

// Signal handler for SIGCHLD
void sigchld_handler(int signo) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("Background process %d finished\n", pid);
    }
    if (pid == -1 && errno != ECHILD) {
        perror("waitpid");
    }
}

int main(int argk, char *argv[], char *envp[]) {
    pid_t frkRtnVal;
    int wpid;
    char *v[NV];
    char *sep = " \t\n";
    int i;
    int background = 0;

    // Set up signal handler for SIGCHLD
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    while (1) {
        prompt();
        if (fgets(line, NL, stdin) == NULL) {
            if (feof(stdin)) {
                fprintf(stderr, "EOF pid %d feof %d ferror %d\n", getpid(), feof(stdin), ferror(stdin));
                exit(0);
            } else {
                perror("fgets");
                exit(EXIT_FAILURE);
            }
        }

        if (ferror(stdin)) {
            perror("ferror on stdin");
            clearerr(stdin);
        }

        if (line[0] == '#' || line[0] == '\n' || line[0] == '\000')
            continue;

        v[0] = strtok(line, sep);
        for (i = 1; i < NV - 1; i++) {  // Leave room for NULL at the end
            v[i] = strtok(NULL, sep);
            if (v[i] == NULL)
                break;
        }
        v[i] = NULL;  // Ensure the argument list is NULL-terminated

        // Check for background process
        background = 0;
        if (i > 0 && strcmp(v[i-1], "&") == 0) {
            background = 1;
            v[i-1] = NULL;  // Remove the '&' from the argument list
        }

        // Handle built-in 'cd' command
        if (v[0] != NULL && strcmp(v[0], "cd") == 0) {
            if (v[1] == NULL) {
                // Change to home directory if no argument is provided
                if (chdir(getenv("HOME")) != 0) {
                    perror("chdir to HOME");
                }
            } else {
                if (chdir(v[1]) != 0) {
                    perror("chdir");
                }
            }
            continue;
        }

        switch (frkRtnVal = fork()) {
        case -1:
            perror("fork");
            break;
        case 0:
            // Child process
            execvp(v[0], v);
            // If execvp returns, it must have failed
            perror("execvp");
            exit(EXIT_FAILURE);  // Terminate child process if exec fails
        default:
            // Parent process
            if (!background) {
                wpid = wait(NULL);
                if (wpid == -1) {
                    perror("wait");
                } else {
                    printf("%s done \n", v[0]);
                }
            } else {
                printf("Started background process %d\n", frkRtnVal);
            }
            break;
        }
    }
}