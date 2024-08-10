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
int bg_count = 0; // Counter for background processes

typedef struct BackgroundProcess {
    int id;
    pid_t pid;
    char command[NL];
} BackgroundProcess;

BackgroundProcess bg_processes[NV];
int bg_index = 0;

void handle_background_processes() {
    int wpid;
    int status;
    while ((wpid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < bg_index; i++) {
            if (bg_processes[i].pid == wpid) {
                fprintf(stdout, "[%d]+ Done\t%s\n", bg_processes[i].id, bg_processes[i].command);
                fflush(stdout);
                bg_processes[i] = bg_processes[--bg_index]; // Remove the finished process from the array
                break;
            }
        }

        // Reset bg_count if no more background processes are running
        if (bg_index == 0) {
            bg_count = 0; // Reset the counter when all background processes are done
        }
    }
}

int main(int argk, char *argv[], char *envp[]) {
    int frkRtnVal;
    char *v[NV];
    char *sep = " \t\n";
    int i;
    int background;

    while (1) {
        // fprintf(stdout, "\n msh> "); // Remove prompt line
        if (fgets(line, NL, stdin) == NULL) {
            if (feof(stdin)) {
                // Exit without additional output on EOF
                exit(0);
            } else {
                perror("fgets error");
                continue;
            }
        }

        if (line[0] == '#' || line[0] == '\n' || line[0] == '\000')
            continue;

        v[0] = strtok(line, sep);
        for (i = 1; i < NV; i++) {
            v[i] = strtok(NULL, sep);
            if (v[i] == NULL)
                break;
        }
        if (i == 0) continue;
        v[i] = NULL;

        background = 0;
        if (i > 1 && strcmp(v[i - 1], "&") == 0) {
            background = 1;
            v[i - 1] = NULL;
            bg_count++; // Increment background process counter
        }

        if (strcmp(v[0], "cd") == 0) {
            if (v[1] == NULL) {
                fprintf(stderr, "cd: missing argument\n");
            } else {
                if (chdir(v[1]) != 0) {
                    perror("cd error");
                } else {
                    setenv("PWD", v[1], 1); // Update PWD environment variable
                }
            }
            continue;
        }

        switch (frkRtnVal = fork()) {
        case -1:
            perror("fork error");
            break;

        case 0:
            execvp(v[0], v);
            perror("execvp error");
            exit(1);

        default:
            if (background) {
                fprintf(stdout, "[%d] %d\n", bg_count, frkRtnVal);
                fflush(stdout);
                // Store background process details for later use
                bg_processes[bg_index].id = bg_count;
                bg_processes[bg_index].pid = frkRtnVal;
                snprintf(bg_processes[bg_index].command, NL, "%s %s", v[0], v[1] ? v[1] : "");
                bg_index++;
            } else {
                if (waitpid(frkRtnVal, NULL, 0) < 0) {
                    perror("waitpid error");
                }
            }

            // Check for completed background processes
            handle_background_processes();

            break;
        }
    }
}