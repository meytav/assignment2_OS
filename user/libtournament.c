#define MAX_PROCESSES 64
#define MAX_LOCKS 63

#include "user.h"

int processID = -1; // Parent doesn't participate in the tournament
int levels;

// Helper functions to calculate role and lock index for each level
int 
getRoleLevel(int level)
{
  return (processID & (1 << (levels - level - 1))) >> (levels - level - 1);
}

int 
getLockLevel(int level)
{
  return (processID >> (levels - level)) + ((1 << level) - 1);
}

// Create all the Peterson locks we'll need
int 
createLocks(int processes)
{
  int num_locks = processes - 1; // Need N-1 locks for N processes
  for (int i = 0; i < num_locks; i++)
  {
    int lock_id = peterson_create();
    if (lock_id < 0)
    {
      printf("Failed to create lock\n");
      tournament_destroy(i);
      return -1;
    }
  }
  return 0;
}


// Calculates number of 1' bits, for n = 2^k, log_2(n) = pop_cnt(n-1) = k.
// Divide and conquer, can be done in-place.
// Based on the book "Hackers delight" by Henry S. Warren, Jr. 
int
pop_cnt(int processes)
{
  int n = (uint) processes;
  n = n - ((n >> 1) & 0x55555555);
  n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
  n = (n + (n >> 4)) & 0x0F0F0F0F;
  n = n + (n >> 8);
  n = n + (n >> 16);
  return (n & 0x0000003F);
}

int 
tournament_create(int processes)
{
  // Check if processes is a power of 2 and ≤ 16
  if ((processes & (processes - 1)) != 0 || processes < 1 || processes > MAX_PROCESSES)
  {
    return -1;
  }

  // Create the Peterson locks
  if (createLocks(processes) < 0)
  {
    return -1;
  }

  levels = pop_cnt(processes - 1);

  // Fork processes and assign IDs - create all N processes (not N-1)
  for (int i = 0; i < processes; i++)
  {
    int pid = fork();
    if (pid < 0)
    {
      tournament_destroy(processes);
      return -1; // Failed to fork
    }
    else if (pid == 0)
    {
      processID = i; // Child process gets ID i (0 to processes)
      return processID;
    }
  }

  return processID; // Parent returns -1 (parent doesn't participate)
}

int 
tournament_acquire(void)
{
  // Acquire locks from bottom to top (leaf to root)
  for (int level = levels - 1; level >= 0; level--)
  {
    if (peterson_acquire(getLockLevel(level), getRoleLevel(level)) < 0)
    {
      return -1;
    }
  }
  return 0;
}

int 
tournament_release(void)
{
  // Release locks from top to bottom (root to leaf)
  for (int level = 0; level < levels; level++)
  {
    if (peterson_release(getLockLevel(level), getRoleLevel(level)) < 0)
    {
      return -1;
    }
  }
  return 0;
}

void
tournament_destroy(int processes)
{
  for (int i = 0; i < processes - 1; i++)
  {
    if (peterson_destroy(i) < 0)
    {
      printf("Failed to destroy tornument");
      exit(1);
    }
  }
}