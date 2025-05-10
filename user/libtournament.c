#include "user.h"
   
// Global lock array (dynamically allocated)
int *locks = 0;

// Maximum number of levels: for N=16 → L = 4
#define MAX_LEVELS 4

// These are specific to the calling process
int my_index;
int my_roles[MAX_LEVELS];
int my_locks[MAX_LEVELS];
int levels; 


int tournament_create(int processes) {
  // Check if processes is a power of 2 and ≤ 16
  if (processes <= 1 || processes > 16) return -1;

  // Check power of 2: only one bit should be set
  if ((processes & (processes - 1)) != 0) return -1;

  // Compute number of levels L = log2(N)
  levels = 0;
  int temp = processes;
  while (temp > 1) {
      levels++;
      temp /= 2;
  }

  // Total number of locks = processes - 1
  int num_locks = processes - 1;
  locks = malloc(sizeof(int) * num_locks);
  if (!locks) return -1;

  for (int i = 0; i < num_locks; i++) {
    int lock_id = peterson_create();  // syscall to kernel
    if (lock_id != i) {
        printf("Error: expected lock ID %d but got %d\n", i, lock_id);
        return -1;
    }
    locks[i] = lock_id;
}


  // Fork N processes and assign each one a tournament ID
  for (int i = 0; i < processes; i++) {
     
    int pid = fork();
      if (pid < 0) {
          return -1; // fork failed
      } else if (pid == 0) {
          // Child process: this is our process
          my_index = i; 
          // Compute lock and role for each level
          for (int l = 0; l < levels; l++) {
              int role = (my_index & (1 << (levels - l - 1))) >> (levels - l - 1);
              int lock_l = my_index >> (levels - l);

              my_roles[l] = role;
              my_locks[l] = lock_l;
          }

          return my_index; // child returns its tournament ID
      }
      // Parent continues looping to fork the rest
  }

  // Parent does not participate
  return -1;
}

int tournament_acquire(void) {
  for (int l = 0; l < levels; l++) {
      int lock_index = my_locks[l] + (1 << l) - 1;
      peterson_acquire(lock_index, my_roles[l]);

  }
  return 0;
}

int tournament_release(void) {
  for (int l = levels - 1; l >= 0; l--) {
      int lock_index = my_locks[l] + (1 << l) - 1;
      peterson_release(lock_index, my_roles[l]);

  }
  return 0;
}

