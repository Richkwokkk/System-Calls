#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

// Signal handler for SIGHUP
void handle_sighup(int sig) {
    printf("Ouch!\n");
}

// Signal handler for SIGINT
void handle_sigint(int sig) {
    printf("Yeah!\n");
}

int main(int argc, char *argv[]) {

    // Check if the correct number of arguments is provided
    if (argc != 2) {
        fprintf(stderr, "Please provide correct number of arguments.\n");
        return 1;
    }

    // Convert the input argument to an integer
    int n = atoi(argv[1]);
    if (n <= 0) {
        fprintf(stderr, "Please provide a positive number.\n");
        return 1;
    }

    // Register signal handlers
    signal(SIGHUP, handle_sighup);
    signal(SIGINT, handle_sigint);

    // Print the first n even numbers with a 5-second sleep between each
    for (int i = 0; i < n; i++) {
        printf("%d\n", i*2);
        sleep(5);
    }

    return 0;
}
