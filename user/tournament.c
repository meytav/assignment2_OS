#include "user.h"

int 
main(int argc, char *argv[]) 
{
    const int processes = 16;
    int tid = tournament_create(processes);
    if (tid == -1) 
    {
        // Parent process or error
        while (wait(0) > 0) {}
        tournament_destroy(processes);
        exit(0);
    }

    // Critical section
    tournament_acquire();
    printf("PID %d, Tournament ID %d\n", getpid(), tid);
    tournament_release();

    exit(0);
}
