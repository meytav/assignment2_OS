#include "user.h"

int main(int argc, char *argv[]) {
    int tid = tournament_create(16);
    if (tid == -1) {
        // Parent process or error
        wait(0); // Wait for children to exit
        exit(0);
    }

    // Critical section
    tournament_acquire();
    printf("PID %d, Tournament ID %d\n", getpid(), tid);
    tournament_release();

    exit(0);
}
